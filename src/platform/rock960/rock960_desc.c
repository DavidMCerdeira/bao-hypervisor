/** 
 * Bao, a Lightweight Static Partitioning Hypervisor 
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      David Cerdeira <davidmcerdeira@gmail.com>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details. 
 *
 */

#include <platform.h>

struct platform_desc platform = {
    .cpu_num = 1, /* ONLY SUPORT A53 FOR NOW, cpu_num max is 4 */
    .region_num = 2,

    /* plat memory is: 0-3.5GB; 4-4.5GB */
    .regions =  (struct mem_region[]) {
        {
            /* 1M-2GB*/
            .base = 0x00100000,
            .size = 0x7C000000,
        },
    },

    .arch = {
        .gic = {
            /* check gic 400 manual */
            .gicd_addr = 0xF8001000,
            .gicc_addr = 0xF8002000,
            .gich_addr = 0xF8004000,
            .gicv_addr = 0xF8006000,
            .maintenance_id = 25
        },

        .generic_timer = {
            .base_addr = 0xFF850000 /* LInux uses this timer....  */
        },
    },

    .console = {
        .base = 0xFF180000 /* UART 2 */
    },
};
