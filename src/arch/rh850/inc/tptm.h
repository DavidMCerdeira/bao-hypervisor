/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __TPTM_H__
#define __TPTM_H__

#include <bao.h>

// We will set TPTMSUDIV to match this frequency (1 MHz, each tick 1us)
#define TPTM_FREQ    (1000000UL)


struct tptm {
    volatile uint32_t TPTMSIRUN;
    volatile uint32_t TPTMSIRRUN;
    volatile uint32_t TPTMSISTP;
    volatile uint32_t TPTMSISTR;
    volatile uint32_t TPTMSIIEN;
    volatile uint32_t TPTMSIUSTR;
    volatile uint32_t TPTMSIDIV;
    uint8_t res0[4];
    volatile uint32_t TPTMSFRUN;
    volatile uint32_t TPTMSFRRUN;
    volatile uint32_t TPTMSFSTP;
    volatile uint32_t TPTMSFSTR;
    volatile uint32_t TPTMSFDIV;
    uint8_t res1[12];
    volatile uint32_t TPTMSURUN;
    volatile uint32_t TPTMSURRUN;
    volatile uint32_t TPTMSUSTP;
    volatile uint32_t TPTMSUSTR;
    volatile uint32_t TPTMSUIEN;
    uint8_t res2[4];
    volatile uint32_t TPTMSUDIV;
    volatile uint32_t TPTMSUTRG;
    uint8_t res3[32];
    volatile uint32_t TPTMSICNT0;
    volatile uint32_t TPTMSILD0;
    volatile uint32_t TPTMSICNT1;
    volatile uint32_t TPTMSILD1;
    uint8_t res43[16];
    volatile uint32_t TPTMSFCNT;
    uint8_t res5[28];
    volatile uint32_t TPTMSUCNT0;
    volatile uint32_t TPTMSUCMP00;
    volatile uint32_t TPTMSUCMP01;
    volatile uint32_t TPTMSUCMP02;
    volatile uint32_t TPTMSUCMP03;
    uint8_t res6[12];
    volatile uint32_t TPTMSUCNT1;
    volatile uint32_t TPTMSUCMP10;
    volatile uint32_t TPTMSUCMP11;
    volatile uint32_t TPTMSUCMP12;
    volatile uint32_t TPTMSUCMP13;
};

extern struct tptm* tptm_hw;

uint32_t tptm_get_count(void);

void tptm_set(uint32_t n);

void tptm_disable(void);

void tptm_init(void);

#endif /* __TPTM_H__ */
