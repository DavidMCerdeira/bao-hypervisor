/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ASM_H__
#define __ASM_H__

/* We must used opcode directly because toolchain does not support instruction */
static inline void stm_gsr(unsigned long addr)
{
    register unsigned long r6 __asm__("r6") = addr;
    __asm__ volatile(".long 0x916007E0 | 6" : : "r"(r6));
}

/* We must used opcode directly because toolchain does not support instruction */
static inline void ldm_gsr(unsigned long addr)
{
    register unsigned long r6 __asm__("r6") = addr;
    __asm__ volatile(".long 0x996007E0 | 6" : : "r"(r6));
}

#define STM_BASE         0x016407E6u
#define LDM_BASE         0x016607E6u

/* Pure constant expression; no casts needed */
#define OPCODE(et, base) (((unsigned)(et) << 27) | (unsigned)(base))

/* ET list 0..31 */
#define ET_LIST_0_31(M)                                                                       \
    M(0)                                                                                      \
    M(1)                                                                                      \
    M(2)                                                                                      \
    M(3)                                                                                      \
    M(4) M(5) M(6) M(7) M(8) M(9) M(10) M(11) M(12) M(13) M(14) M(15) M(16) M(17) M(18) M(19) \
        M(20) M(21) M(22) M(23) M(24) M(25) M(26) M(27) M(28) M(29) M(30) M(31)

#define DECL_STM(ET)                                                                     \
    static inline void stm_mp##ET(unsigned long addr)                                    \
    {                                                                                    \
        register unsigned long r6 __asm__("r6") = addr;                                  \
        __asm__ volatile(".long ((" XSTR(ET) " << 27) | " XSTR(STM_BASE) ")" : : "r"(r6) \
                         : "memory");                                                    \
    }

#define DECL_LDM(ET)                                                                     \
    static inline void ldm_mp##ET(unsigned long addr)                                    \
    {                                                                                    \
        register unsigned long r6 __asm__("r6") = addr;                                  \
        __asm__ volatile(".long ((" XSTR(ET) " << 27) | " XSTR(LDM_BASE) ")" : : "r"(r6) \
                         : "memory");                                                    \
    }

ET_LIST_0_31(DECL_STM)
ET_LIST_0_31(DECL_LDM)

#undef DECL_STM
#undef DECL_LDM

static inline void stm_mp_dispatch(unsigned et, unsigned long addr)
{
    switch (et) {
#define CASE_STM(ET)      \
    case (ET):            \
        stm_mp##ET(addr); \
        break;
        CASE_STM(0);
        CASE_STM(1);
        CASE_STM(2);
        CASE_STM(3);
        CASE_STM(4);
        CASE_STM(5);
        CASE_STM(6);
        CASE_STM(7);
        CASE_STM(8);
        CASE_STM(9);
        CASE_STM(10);
        CASE_STM(11);
        CASE_STM(12);
        CASE_STM(13);
        CASE_STM(14);
        CASE_STM(15);
        CASE_STM(16);
        CASE_STM(17);
        CASE_STM(18);
        CASE_STM(19);
        CASE_STM(20);
        CASE_STM(21);
        CASE_STM(22);
        CASE_STM(23);
        CASE_STM(24);
        CASE_STM(25);
        CASE_STM(26);
        CASE_STM(27);
        CASE_STM(28);
        CASE_STM(29);
        CASE_STM(30);
        CASE_STM(31);
#undef CASE_STM
        default:
            /* handle invalid et however you want */
            __asm__ volatile("" : : : "memory"); /* or trap/assert */
            break;
    }
}

static inline void ldm_mp_dispatch(unsigned et, unsigned long addr)
{
    switch (et) {
#define CASE_LDM(ET)      \
    case (ET):            \
        ldm_mp##ET(addr); \
        break;
        CASE_LDM(0);
        CASE_LDM(1);
        CASE_LDM(2);
        CASE_LDM(3);
        CASE_LDM(4);
        CASE_LDM(5);
        CASE_LDM(6);
        CASE_LDM(7);
        CASE_LDM(8);
        CASE_LDM(9);
        CASE_LDM(10);
        CASE_LDM(11);
        CASE_LDM(12);
        CASE_LDM(13);
        CASE_LDM(14);
        CASE_LDM(15);
        CASE_LDM(16);
        CASE_LDM(17);
        CASE_LDM(18);
        CASE_LDM(19);
        CASE_LDM(20);
        CASE_LDM(21);
        CASE_LDM(22);
        CASE_LDM(23);
        CASE_LDM(24);
        CASE_LDM(25);
        CASE_LDM(26);
        CASE_LDM(27);
        CASE_LDM(28);
        CASE_LDM(29);
        CASE_LDM(30);
        CASE_LDM(31);
#undef CASE_LDM
        default:
            __asm__ volatile("" : : : "memory");
            break;
    }
}

#endif /* __ASM_H__ */
