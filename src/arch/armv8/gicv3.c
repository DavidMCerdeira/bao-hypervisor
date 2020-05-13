/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *      Angelo Ruocco <angeloruocco90@gmail.com>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#include <arch/gic.h>
#include <arch/sysregs.h>
#include <cpu.h>
#include <mem.h>
#include <platform.h>
#include <interrupts.h>
#include <fences.h>

volatile gicd_t gicd __attribute__((section(".devices")));
volatile gicr_t gicr __attribute__((section(".cpu_private_devices")));

volatile gich_t gich __attribute__((section(".devices"), aligned(PAGE_SIZE))); //TODO REMOVE

static spinlock_t gicd_lock;

uint64_t NUM_LRS;


static inline uint64_t gich_num_lrs()
{
    return ((MRS(ICH_VTR_EL2) & GICH_VTR_MSK) >> GICH_VTR_OFF) + 1;
}

static inline uint32_t gich_read_lr(size_t i){

    uint32_t val = 0;

    switch(i) {
        case 0:  val = MRS(FICH_LR0_EL2); break;
        case 1:  val = MRS(FICH_LR1_EL2); break;
        case 2:  val = MRS(FICH_LR2_EL2); break;
        case 3:  val = MRS(FICH_LR3_EL2); break;
        case 4:  val = MRS(FICH_LR4_EL2); break;
        case 5:  val = MRS(FICH_LR5_EL2); break;
        case 6:  val = MRS(FICH_LR6_EL2); break;
        case 7:  val = MRS(FICH_LR7_EL2); break;
        case 8:  val = MRS(FICH_LR8_EL2); break;
        case 9:  val = MRS(FICH_LR9_EL2); break;
        case 10: val = MRS(FICH_LR10_EL2); break;
        case 11: val = MRS(FICH_LR11_EL2); break;
        case 12: val = MRS(FICH_LR12_EL2); break;
        case 13: val = MRS(FICH_LR13_EL2); break;
        case 14: val = MRS(FICH_LR14_EL2); break;
        case 15: val = MRS(FICH_LR15_EL2); break;
    }
}

static inline void gich_write_lr(size_t i, uint32_t val){

    switch(i) {
        case 0:  MSR(ICH_LR0_EL2, val); break;
        case 1:  MSR(ICH_LR1_EL2, val); break;
        case 2:  MSR(ICH_LR2_EL2, val); break;
        case 3:  MSR(ICH_LR3_EL2, val); break;
        case 4:  MSR(ICH_LR4_EL2, val); break;
        case 5:  MSR(ICH_LR5_EL2, val); break;
        case 6:  MSR(ICH_LR6_EL2, val); break;
        case 7:  MSR(ICH_LR7_EL2, val); break;
        case 8:  MSR(ICH_LR8_EL2, val); break;
        case 9:  MSR(ICH_LR9_EL2, val); break;
        case 10: MSR(ICH_LR10_EL2, val); break;
        case 11: MSR(ICH_LR11_EL2, val); break;
        case 12: MSR(ICH_LR12_EL2, val); break;
        case 13: MSR(ICH_LR13_EL2, val); break;
        case 14: MSR(ICH_LR14_EL2, val); break;
        case 15: MSR(ICH_LR15_EL2, val); break;
    }
}

static inline void gicc_init()
{
    /* Enable system register interface i*/
    MSR(ICC_SRE_EL2, ICC_SRE_SRE_BIT);
    ISB();

    for (int i = 0; i < gich_num_lrs(); i++) {
        gich_write_lr(i, 0);
    }

    MSR(ICC_PMR_EL1, -1);
    MSR(ICC_CTLR_EL1, GICC_CTLR_EN_BIT | GICC_CTLR_EOImodeNS_BIT);
    MSR(ICH_HCR_EL2, MRS(ICH_HCR_EL2) | GICH_HCR_LRENPIE_BIT);
    MSR(ICC_IGRPEN1_EL1, ICC_IGRPEN_EL1_ENB_BIT);
}

static inline void gicr_init()
{
    mem_map_dev(&cpu.as, (void *)&gicr,
                platform.arch.gic.gicr_addr + (cpu.id * sizeof(gicr)),
                NUM_PAGES(sizeof(gicr)));

    gicr.ICENABLER0 = -1;
    gicr.ICPENDR0 = -1;
    gicr.ICACTIVER0 = -1;

    for (int i = 0; i < GIC_NUM_PRIO_REGS(GIC_CPU_PRIV); i++) {
        gicr.IPRIORITYR[i] = -1;
    }
}


void gicc_save_state(gicc_state_t *state)
{
    // state->CTLR = gicc.CTLR;
    // state->PMR = gicc.PMR;
    // state->BPR = gicc.BPR;
    // state->IAR = gicc.IAR;
    // state->EOIR = gicc.EOIR;
    // state->RPR = gicc.RPR;
    // state->HPPIR = gicc.HPPIR;
    // state->priv_ISENABLER = gicd.ISENABLER[0];

    // for (int i = 0; i < GIC_NUM_PRIO_REGS(GIC_CPU_PRIV); i++) {
    //     state->priv_IPRIORITYR[i] = gicd.IPRIORITYR[i];
    // }

    // state->HCR = gich.HCR;
    // for (int i = 0; i < gich_num_lrs(); i++) {
    //     state->LR[i] = gich.LR[i];
    // }
}

void gicc_restore_state(gicc_state_t *state)
{
    // gicc.CTLR = state->CTLR;
    // gicc.PMR = state->PMR;
    // gicc.BPR = state->BPR;
    // gicc.IAR = state->IAR;
    // gicc.EOIR = state->EOIR;
    // gicc.RPR = state->RPR;
    // gicc.HPPIR = state->HPPIR;
    // gicd.ISENABLER[0] = state->priv_ISENABLER;

    // for (int i = 0; i < GIC_NUM_PRIO_REGS(GIC_CPU_PRIV); i++) {
    //     gicd.IPRIORITYR[i] = state->priv_IPRIORITYR[i];
    // }

    // gich.HCR = state->HCR;
    // for (int i = 0; i < gich_num_lrs(); i++) {
    //     gich.LR[i] = state->LR[i];
    // }
}


void gic_cpu_init()
{
    gicr_init();
    gicc_init();
}

void gicd_init(){

    mem_map_dev(&cpu.as, (void *)&gicd, platform.arch.gic.gicd_addr,
                NUM_PAGES(sizeof(gicd)));

    size_t int_num = gic_num_irqs();

    /* Bring distributor to known state */
    for (int i = GIC_NUM_PRIVINT_REGS; i < GIC_NUM_INT_REGS(int_num); i++) {
        /**
         * Make sure all interrupts are not enabled, non pending,
         * non active.
         */
        gicd.ICENABLER[i] = -1;
        gicd.ICPENDR[i] = -1;
        gicd.ICACTIVER[i] = -1;
    }

    /* All interrupts have lowest priority possible by default */
    for (int i = GIC_CPU_PRIV; i < GIC_NUM_PRIO_REGS(int_num); i++)
        gicd.IPRIORITYR[i] = -1;

    /* No CPU targets for any interrupt by default */
    for (int i = GIC_CPU_PRIV; i < GIC_NUM_TARGET_REGS(int_num); i++)
        gicd.ITARGETSR[i] = 0;

    /* ICFGR are platform dependent, lets leave them as is */

    /* No need to setup gicd.NSACR as all interrupts are  setup to group 1 */

    interrupts_reserve(platform.arch.gic.maintenance_id,
                    gic_maintenance_handler);

    /* Enable distributor and affinity routing */
    gicd.CTLR |= GICD_CTLR_ARE_NS_BIT | GICD_CTLR_ENA_BIT;
}

void gic_init()
{
    gic_cpu_init();

    if(cpu.id == CPU_MASTER){
        gicd_init();
        NUM_LRS = gich_num_lrs();
    }

    cpu_sync_barrier(&cpu_glb_sync);
}

void gic_handle()
{
    uint64_t ack = MRS(ICC_IAR1_EL1);
    uint64_t id = bit_extract(ack, 0, 24);

    if (id >= 1022) return;

    enum irq_res res = interrupts_handle(id, 0);

    MSR(ICC_EOIR1_EL1, ack);

    if (res == HANDLED_BY_HYP) MSR(ICC_DIR_EL1, ack);
}


void gicd_send_sgi(uint64_t cpu_target, uint64_t sgi_num)
{
    MSR(ICC_SGI1R_EL1, (1UL << cpu_target) | (sgi_num) << 24);
}

uint64_t gicd_get_prio(uint64_t int_id)
{
    uint64_t reg_ind = GIC_PRIO_REG(int_id);
    uint64_t off = GIC_PRIO_OFF(int_id);

    spin_lock(&gicd_lock);

    uint64_t prio =
        gicd.IPRIORITYR[reg_ind] >> off & BIT_MASK(off, GIC_PRIO_BITS);

    spin_unlock(&gicd_lock);

    return prio;
}

void gicd_set_icfgr(uint64_t int_id, uint8_t cfg)
{
    // spin_lock(&gicd_lock);

    // uint64_t reg_ind = (int_id*GIC_CONFIG_BITS)/(sizeof(uint32_t)*8);
    // uint64_t off = (int_id*GIC_CONFIG_BITS)%(sizeof(uint32_t)*8);
    // uint64_t mask = ((1U << GIC_CONFIG_BITS) - 1) << off;

    // gicd.ICFGR[reg_ind] =
    //     (gicd.ICFGR[reg_ind] & ~mask) | ((cfg << off) & mask);

    // spin_unlock(&gicd_lock);
}

void gicd_set_prio(uint64_t int_id, uint8_t prio)
{
    uint64_t reg_ind = GIC_PRIO_REG(int_id);
    uint64_t off = GIC_PRIO_OFF(int_id);
    uint64_t mask = BIT_MASK(off, GIC_PRIO_BITS);


    spin_lock(&gicd_lock);

    if(gic_is_priv(int_id)){
        gicr.IPRIORITYR[reg_ind] =
            (gicr.IPRIORITYR[reg_ind] & ~mask) | ((prio << off) & mask);
    } else {
        gicd.IPRIORITYR[reg_ind] =
            (gicd.IPRIORITYR[reg_ind] & ~mask) | ((prio << off) & mask);
    }

    spin_unlock(&gicd_lock);
}

enum int_state gicd_get_state(uint64_t int_id)
{
    // uint64_t reg_ind = GIC_INT_REG(int_id);
    // uint64_t mask = GIC_INT_MASK(int_id);

    // spin_lock(&gicd_lock);

    // enum int_state pend = (gicd.ISPENDR[reg_ind] & mask) ? PEND : 0;
    // enum int_state act = (gicd.ISACTIVER[reg_ind] & mask) ? ACT : 0;

    // spin_unlock(&gicd_lock);

    // return pend | act;
}

static void gicd_set_pend(uint64_t int_id, bool pend)
{
    // spin_lock(&gicd_lock);

    // if (gic_is_sgi(int_id)) {
    //     uint64_t reg_ind = GICD_SGI_REG(int_id);
    //     uint64_t off = GICD_SGI_OFF(int_id);

    //     if (pend) {
    //         gicd.SPENDSGIR[reg_ind] = (1U) << (off + cpu.id);
    //     } else {
    //         gicd.CPENDSGIR[reg_ind] = BIT_MASK(off, 8);
    //     }
    // } else {
    //     uint64_t reg_ind = GIC_INT_REG(int_id);

    //     if (pend) {
    //         gicd.ISPENDR[reg_ind] = GIC_INT_MASK(int_id);
    //     } else {
    //         gicd.ICPENDR[reg_ind] = GIC_INT_MASK(int_id);
    //     }
    // }

    // spin_unlock(&gicd_lock);
}

void gicd_set_act(uint64_t int_id, bool act)
{
    // uint64_t reg_ind = GIC_INT_REG(int_id);

    // spin_lock(&gicd_lock);

    // if (act) {
    //     gicd.ISACTIVER[reg_ind] = GIC_INT_MASK(int_id);
    // } else {
    //     gicd.ICACTIVER[reg_ind] = GIC_INT_MASK(int_id);
    // }

    // spin_unlock(&gicd_lock);
}

void gicd_set_state(uint64_t int_id, enum int_state state)
{
    // gicd_set_act(int_id, state & ACT);
    // gicd_set_pend(int_id, state & PEND);
}

void gicd_set_route(uint64_t int_id, uint8_t trgt)
{
    if(gic_is_priv(int_id))
        return;

    gicd.IROUTER[int_id] = trgt;
}

void gicd_set_enable(uint64_t int_id, bool en)
{
    uint64_t bit = GIC_INT_MASK(int_id);

    if(gic_is_priv(int_id)){
        if (en) gicr.ISENABLER0 = bit;
        else gicr.ICENABLER0 = bit;
    } else {
        uint64_t reg_ind = GIC_INT_REG(int_id);
        spin_lock(&gicd_lock);
        if (en) gicd.ISENABLER[reg_ind] = bit;
        else gicd.ICENABLER[reg_ind] = bit;
        spin_unlock(&gicd_lock);
    }

}
