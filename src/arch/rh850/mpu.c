/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <bao.h>
#include <cpu.h>
#include <arch/mpu.h>
#include <srs.h>
#include <arch/fences.h>
#include <bitmap.h>
#include <plat/pbg.h>
#include <arch/vmm.h>
#include <asm.h>

static inline size_t mpu_num_entries(void)
{
    unsigned long nmpue = MPCFG_GET_NMPUE(srs_mpcfg_read());
    unsigned long num = nmpue + 1;
    return num;
}

static void mpu_entry_set(struct addr_space* as, mpid_t mpid, struct mp_region* mpr)
{
    unsigned long lim = mpr->base + mpr->size - 4;

    if(as->type != AS_VM){
        /* update the physical MPU for hypervisor entries */
        srs_mpidx_write(mpid & MPIDX_IDX_MASK);
        srs_mpla_write(mpr->base & MPLA_MASK);
        srs_mpua_write(lim & MPUA_MASK);
        srs_mpat_write(mpr->mem_flags.raw);
        synci();
    } else {
        /* update the vcpu MPU for VM entries */
        unsigned long last_idx = as->arch.vcpu_mpu_entries[srs_peid_read()].last_idx;
        as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[last_idx].MPLA = mpr->base & MPLA_MASK;
        as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[last_idx].MPUA = lim & MPUA_MASK;
        as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[last_idx].MPAT = mpr->mem_flags.raw;
        last_idx--;
        as->arch.vcpu_mpu_entries[srs_peid_read()].last_idx = last_idx;
    }

}

static void mpu_entry_clear(mpid_t mpid)
{
    srs_mpidx_write(mpid & MPIDX_IDX_MASK);
    srs_mpla_write(0);
    srs_mpua_write(0);
    srs_mpat_write(0);

    synci();
}

static mpid_t mpu_entry_allocate(struct addr_space* as)
{
    mpid_t reg_num = INVALID_MPID;
    for (mpid_t i = (mpid_t)mpu_num_entries(); i > 0; i--) {
        /* mpid_t is unsigned so we can't test for negative number.
         * Subtract 1 to get the index */
        mpid_t idx = i - 1;
        if (bitmap_get(cpu()->arch.mpu_hyp.bitmap, idx) == 0) {
            reg_num = idx;
            break;
        }
    }

    /* For VM mpu entry allocations we don't reserve the mpu entry on the hyp
     * bitmap because other VMs can reause the same entry upon a context
     * switch. Instead we check the allocated for this vcpu. */
    if (as->type != AS_VM) {
        bitmap_set(cpu()->arch.mpu_hyp.bitmap, reg_num);
    } else {
        for (mpid_t i = reg_num + 1; i > 0; i--) {
            mpid_t idx = i - 1;
            if (bitmap_get(as->arch.vcpu_mpu_entries[srs_peid_read()].vcpu_allocated_entries, idx) == 0) {
                bitmap_set(as->arch.vcpu_mpu_entries[srs_peid_read()].vcpu_allocated_entries, idx);
                reg_num = idx;
                break;
            }
        }
    }
    return reg_num;
}

static inline void mpu_set_hbe(unsigned long hbe)
{
    unsigned long mpcfg = srs_mpcfg_read();
    mpcfg = (mpcfg & ~MPCFG_HBE_MASK) | (hbe << MPCFG_HBE_OFF);
    srs_mpcfg_write(mpcfg);
}

static inline unsigned long mpu_get_hbe(void)
{
    unsigned long mpcfg = srs_mpcfg_read();
    unsigned long hbe = ((mpcfg & MPCFG_HBE_MASK) >> MPCFG_HBE_OFF);
    return hbe;
}

static inline void update_hbe(void)
{
    /* Hyp entries are allocated top to bottom. By finding the first
     * entry from 0 to end, we get the last entry used by the hyp. This is
     * then used as the watermark */
    mpid_t mpid =
        (mpid_t)bitmap_find_nth(cpu()->arch.mpu_hyp.bitmap, mpu_num_entries(), 1, 0, BITMAP_SET);
    mpu_set_hbe(mpid);
}

bool mpu_add_region(struct addr_space* as, struct mp_region* reg, bool locked)
{
    UNUSED_ARG(locked);

    if (reg->size > 0) {
        mpid_t mpid = 0;
        mpid = mpu_entry_allocate(as);

        if (mpid != INVALID_MPID) {
            mpu_entry_set(as, mpid, reg);
            update_hbe();
        }
    }

    return false;
}

static void mpu_entry_get_region(struct addr_space* as, mpid_t mpid, struct mp_region* mpe)
{
    unsigned long base = 0;
    unsigned long limit = 0;
    unsigned long flags;
    if (as->type != AS_VM) { /* HYP */
        srs_mpidx_write(mpid & MPIDX_IDX_MASK);
        syncp();

        base = srs_mpla_read();
        limit = srs_mpua_read();
        flags = srs_mpat_read();
    } else {
        base = as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[mpid].MPLA;
        limit = as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[mpid].MPUA;
        flags = as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[mpid].MPAT;
    }
    mpe->base = base;
    mpe->size = (limit - base) + 4;
    mpe->mem_flags.raw = (uint16_t)flags;
    mpe->as_sec = SEC_UNKNOWN;
}

static mpid_t mpu_entry_get_region_id(struct addr_space* as, struct mp_region* mem)
{
    mpid_t mpid = INVALID_MPID;

    if (as->type != AS_VM) { /* HYP */
        for (mpid_t i = 0; i < (mpid_t)mpu_num_entries(); i++) {
            if (bitmap_get(cpu()->arch.mpu_hyp.bitmap, i)) {
                struct mp_region mpe_cmp;
                mpu_entry_get_region(as, i, &mpe_cmp);

                if (mpe_cmp.base == mem->base && mpe_cmp.size == mem->size) {
                    if (mpe_cmp.mem_flags.wmpid7 || mpe_cmp.mem_flags.rmpid7) {
                        mpid = i;
                        break;
                    }
                }
            }
        }
    } else { /* VM */
        for (mpid_t i = 0; i < (mpid_t)mpu_num_entries(); i++) {
            if (bitmap_get(as->arch.vcpu_mpu_entries[srs_peid_read()].vcpu_allocated_entries, i)) {
                struct mp_region mpe_cmp;
                mpu_entry_get_region(as, i, &mpe_cmp);
                if (mpe_cmp.base == mem->base && mpe_cmp.size == mem->size) {
                    if (mpe_cmp.mem_flags.wmpid6 || mpe_cmp.mem_flags.rmpid6) {
                        mpid = i;
                        break;
                    }
                }
            }
        }
    }

    return mpid;
}

static inline void mpu_entry_free(mpid_t mpid)
{
    mpu_entry_clear(mpid);
    bitmap_clear(cpu()->arch.mpu_hyp.bitmap, mpid);
}

bool mpu_remove_region(struct addr_space* as, struct mp_region* mem)
{
    bool failed = true;

    if (mem->size > 0) {
        mpid_t mpid = mpu_entry_get_region_id(as, mem);

        if (mpid != INVALID_MPID) {
            failed = false;
            mpu_entry_free(mpid);
            update_hbe();
        }
    }

    return !failed;
}

bool mpu_update_region(struct addr_space* as, struct mp_region* mpr)
{
    mpid_t mpid = mpu_entry_get_region_id(as, mpr);
    if (mpid != INVALID_MPID) {
        mpu_entry_set(as, mpid, mpr);
        update_hbe();
        return true;
    }

    return false;
}

static inline bool mpu_entry_valid(mpid_t mpid)
{
    srs_mpidx_write(mpid & MPIDX_IDX_MASK);
    syncp();

    unsigned long valid_bit = MPAT_GET_E(srs_mpat_read());

    return !!valid_bit;
}

void mpu_arch_init(void)
{
    bitmap_clear_consecutive(cpu()->arch.mpu_hyp.bitmap, 0, mpu_num_entries());

    for (mpid_t mpid = 0; mpid < (mpid_t)mpu_num_entries(); mpid++) {
        /* No entry should be valid at this point */
        if (mpu_entry_valid(mpid)) {
            bitmap_clear(cpu()->arch.mpu_hyp.bitmap, mpid);
        }
    }

    /* give no mpu entries to hypervisor, we will allocate them dynamically */
    mpu_set_hbe(mpu_num_entries());

    if (cpu_is_master()) {
        /* Initialize slave side protection.
         * In the future we should figure out how to move this to io_init */
        pbg_init();
    }

    srs_mpid7_write(HYP_SPID);
    srs_spid_write(HYP_SPID);
}

void mpu_arch_enable(void)
{
    srs_mpm_write(MPM_SVP | MPM_MPE);
    synci();
}

void mpu_arch_disable(void)
{
    unsigned long mpm = srs_mpm_read() & ~MPM_MPE;
    srs_mpm_write(mpm);
}

void mpu_restore_state(struct addr_space* as)
{
    unsigned long et = mpu_get_hbe();
    as->arch.vcpu_mpu_entries[srs_peid_read()].save_hbe = et;
    unsigned n = 31-et+1;
    unsigned long addr = (unsigned long)&(as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[n]);
    mpu_set_hbe(0);
    ldm_mp_dispatch(et-1, addr);
}

void mpu_save_state(struct addr_space* as)
{
    unsigned hbe = as->arch.vcpu_mpu_entries[srs_peid_read()].save_hbe;
    unsigned long et = hbe;
    unsigned n = 31-et+1;
    unsigned long addr = (unsigned long)&(as->arch.vcpu_mpu_entries[srs_peid_read()].mpu_entries[n]);
    stm_mp_dispatch(et-1, addr);
    mpu_set_hbe(hbe);
}
