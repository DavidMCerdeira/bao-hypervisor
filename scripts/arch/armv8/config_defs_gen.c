/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved
 */

#include <stdio.h>
#include <config.h>
#include <platform.h>
#include <arch/gic.h>

/**
 * Number of interrupt descriptors the virtual GIC of a vm needs. Must mirror vgic_get_itln(): the
 * vm's own interrupt_num when set, otherwise the platform GIC's line count, otherwise the
 * architecture maximum, rounded up to 32.
 */
static size_t vm_vgic_int_num(const struct vm_config* vm_config)
{
    size_t int_num = vm_config->platform.arch.gic.interrupt_num;
    if (int_num <= GIC_MAX_PPIS) {
        int_num = platform.arch.gic.interrupt_num;
    }
    if (int_num <= GIC_MAX_PPIS) {
        int_num = GIC_MAX_INTERUPTS;
    }
    return ALIGN(int_num, 32);
}

/* Every vm embeds its own interrupt descriptor array, sized for the largest vm */
void arch_config_defs(void)
{
    size_t vgic_int_num = 0;
    for (size_t i = 0; i < config.vmlist_size; i++) {
        size_t vm_int_num = vm_vgic_int_num(&config.vmlist[i]);
        if (vm_int_num > vgic_int_num) {
            vgic_int_num = vm_int_num;
        }
    }

    printf("#define CONFIG_VGIC_INT_NUM %ld\n", vgic_int_num);
}
