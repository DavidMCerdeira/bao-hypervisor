#ifndef TIMER_ARCH_H
#define TIMER_ARCH_H

#include <bao.h>
#include <arch/cpu.h>
#include <tptm.h>

typedef uint64_t timer_value_t;

#define TIMER_ARCH_FREQ() (1000000UL)

#define HYP_TIMER_IRQ_ID  (209)

static irqid_t timer_arch_irq_id(void);
static inline irqid_t timer_arch_irq_id(void)
{
    return (irqid_t)HYP_TIMER_IRQ_ID;
}

static uint64_t timer_arch_get_count(void);
static inline uint64_t timer_arch_get_count()
{
    return (uint64_t)tptm_get_count();
}

static void timer_arch_set(uint64_t n);
static inline void timer_arch_set(uint64_t n)
{
    UNUSED_ARG(n);
    /* tptm_set((uint32_t)n); */
}

static void timer_arch_disable(void);
static inline void timer_arch_disable(void)
{
    tptm_disable();
}

static void timer_arch_init(void);
static inline void timer_arch_init(void)
{
    /* tptm_init(); */
    /* timer_arch_disable(); */
}

#endif /* TIMER_ARCH_H */
