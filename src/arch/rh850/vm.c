/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <config.h>
#include <string.h>
#include <vm.h>
#include <vintc.h>
#include <platform.h>
#include <vipir.h>
#include <vbootctrl.h>
#include <asm.h>
#include <arch/mpu.h>

void vm_arch_init(struct vm* vm, const struct vm_config* vm_config)
{
    UNUSED_ARG(vm_config);

    /* All VMs use MPID6 for memory protection */
    srs_mpid6_write(vm->id);

    vintc_init(vm);
    vipir_init(vm);
    vbootctrl_init(vm);
}

void vcpu_arch_init(struct vcpu* vcpu, struct vm* vm)
{
    UNUSED_ARG(vm);
    UNUSED_ARG(vcpu);

    vm->as.arch.vcpu_mpu_entries[srs_peid_read()].last_idx = 31;
}

void vcpu_arch_reset(struct vcpu* vcpu, vaddr_t entry)
{
    struct vm* vm = vcpu->vm;

    memset(&vcpu->regs, 0, sizeof(struct arch_regs));

    vcpu_writepc(vcpu, entry);
    srs_eipc_write(entry);

    vcpu->arch.started = vcpu->id == 0 ? true : false;

    /* Bao fixes the VMID as SPID to isolate VM memory regions */
    srs_gmspid_write(vm->id);
    srs_gmspidlist_write(0x0);

    srs_gmmpm_write(GMMPM_GMPE);

    unsigned long eipswh = srs_eipswh_read() & ~EIPSWH_GPID_MASK;
    srs_eipswh_write(eipswh | (vm->id << EIPSWH_GPID_OFF));

    unsigned long fepswh = srs_fepswh_read() & ~FEPSWH_GPID_MASK;
    srs_fepswh_write(fepswh | (vm->id << FEPSWH_GPID_OFF));

    srs_gmpeid_write(vcpu->id);

    /* clear guest-context exception registers */
    srs_gmeipc_write(0x0);
    srs_gmfepc_write(0x0);
    srs_gmmea_write(0x0);
    srs_gmmei_write(0x0);
    srs_gmeiic_write(0x0);
    srs_gmfeic_write(0x0);

    /* TODO */
    vcpu->regs.sys_regs.RBCR1 = 0xffff;
    vcpu->regs.sys_regs.GMEIPSW = 0x3F00002;
    vcpu->regs.sys_regs.GMFEPSW = 0x20;
    vcpu->regs.sys_regs.GMPSW = 0x8020;
    vcpu->regs.sys_regs.GMINTCFG = 0xf0002;
    vcpu->regs.sys_regs.GMPLMR = 0x3f;
    vcpu->regs.sys_regs.GMMPM = GMMPM_GMPE; /* critical for enabling guest access control */
    vcpu->regs.sys_regs.MCA = srs_mca_read();
    vcpu->regs.sys_regs.MCS = srs_mcs_read();
    vcpu->regs.sys_regs.MCR = srs_mcr_read();
    vcpu->regs.sys_regs.MCI = srs_mci_read();

    srs_tsctrl_write(1);
    vintc_vcpu_reset(vcpu);
}

bool vcpu_arch_is_on(struct vcpu* vcpu)
{
    return vcpu->arch.started;
}

unsigned long vcpu_readreg(struct vcpu* vcpu, unsigned long reg)
{
    if (reg > 32) {
        ERROR("reading register out of bounds");
    }

    /* r0 is always 0x0 */
    return reg == 0 ? 0 : vcpu->regs.gp_regs.r[reg];
}

void vcpu_writereg(struct vcpu* vcpu, unsigned long reg, unsigned long val)
{
    if (reg > 32) {
        ERROR("writing register out of bounds");
    }

    /* r0 is always 0x0 */
    if (reg != 0) {
        vcpu->regs.gp_regs.r[reg] = val;
    }
}

unsigned long vcpu_readpc(struct vcpu* vcpu)
{
    return vcpu->regs.pc;
}

void vcpu_writepc(struct vcpu* vcpu, unsigned long val)
{
    vcpu->regs.pc = val;
}

void vcpu_restore_state(struct vcpu* vcpu)
{
    const struct arch_regs* r = &vcpu->regs;
    /* TODO: MPU regions */
    ldm_gsr((unsigned long)&(vcpu->regs.sys_regs));

    srs_eipc_write(r->pc);

    mpu_restore_state(&vcpu->vm->as);

    vmid_t id = vcpu->vm->id;
    unsigned long eipswh = srs_eipswh_read() & ~EIPSWH_GPID_MASK;
    srs_eipswh_write(eipswh | (id << EIPSWH_GPID_OFF));

    unsigned long fepswh = srs_fepswh_read() & ~FEPSWH_GPID_MASK;
    srs_fepswh_write(fepswh | (id << FEPSWH_GPID_OFF));

    srs_gmspid_write(vcpu->vm->id);
    srs_mpid6_write(vcpu->vm->id);

    /* unsigned long pswh = get_pswh() & ~EIPSWH_GPID_MASK; */
    /* set_pswh(vcpu->vm->id); */
    /* set_pswh(pswh | (vcpu->vm->id << EIPSWH_GPID_OFF)); */
}

void vcpu_save_state(struct vcpu* vcpu)
{
    (void)vcpu;
    stm_gsr((unsigned long)&(vcpu->regs.sys_regs));

    /* TODO: MPU regions */
    mpu_save_state(&vcpu->vm->as);
}
