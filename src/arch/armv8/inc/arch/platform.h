/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ARCH_PLATFORM_H__
#define __ARCH_PLATFORM_H__

#include <bao.h>
#include <arch/gic.h>
#ifdef MEM_PROT_MMU
#include <arch/smmuv2.h>
#endif

struct arch_platform {
    struct gic_dscrp {
        paddr_t gicc_addr;
        paddr_t gich_addr;
        paddr_t gicv_addr;
        paddr_t gicd_addr;
        paddr_t gicr_addr;

        irqid_t maintenance_id;
        /**
         * Number of interrupt lines of the GIC (32 * (ITLinesNumber + 1)). Bounds the lines a
         * vm's virtual GIC exposes when the vm configuration does not set its own interrupt_num,
         * and sizes the static storage of the virtual GIC interrupt descriptors.
         */
        size_t interrupt_num;
    } gic;

#ifdef MEM_PROT_MMU
    struct {
        paddr_t base;
        irqid_t interrupt_id;
        streamid_t global_mask;
    } smmu;
#endif

    struct {
        paddr_t base_addr;
        uint32_t fixed_freq;
    } generic_timer;

    struct clusters {
        size_t num;
        size_t* core_num;
    } clusters;

    struct mpidr_table {
        unsigned long* table;
    } mpidr_table;
};

struct platform;
unsigned long platform_arch_cpuid_to_mpidr(const struct platform* plat, cpuid_t cpuid);
#endif /* __ARCH_PLATFORM_H__ */
