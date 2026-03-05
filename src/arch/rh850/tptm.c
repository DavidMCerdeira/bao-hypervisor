/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <tptm.h>
#include <mem.h>
#include <cpu.h>
#include <intc.h>

struct tptm* tptm_hw;

uint32_t tptm_get_count()
{
    return tptm_hw->TPTMSUCNT0;
}

void tptm_set(uint32_t n)
{
    tptm_hw->TPTMSUSTP = 1;
    tptm_hw->TPTMSURUN = 1;
    tptm_hw->TPTMSUCMP00 = n;
}

void tptm_disable(void)
{
    tptm_hw->TPTMSUIEN = 0; // disable interrupt to channel 0, compare value 0
}

void tptm_init(void)
{
    vaddr_t tptm_ptr = mem_alloc_map_dev(&cpu()->as, SEC_HYP_PRIVATE, INVALID_VA,
        platform.arch.tptm_addr, NUM_PAGES(sizeof(struct tptm)));
    if (tptm_ptr == INVALID_VA) {
        ERROR("maping tptm failed");
    }
    tptm_hw = (struct tptm*)tptm_ptr;

    intc_set_tptmsel(TPTM_SEL_EIINT); // configure interrupt as ei
    tptm_hw->TPTMSUIEN = (1ULL << 0); // enable interrupt to channel 0, compare value 0
    tptm_hw->TPTMSUDIV = (PLAT_CLK_CPU / TPTM_FREQ) - 1;
    tptm_hw->TPTMSUCMP00 = ~0UL;      // set compare value 0 to maximum in channel 0
    tptm_hw->TPTMSURUN = 1;           // load counter with 0 in channel 0
}
