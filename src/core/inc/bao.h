/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __BAO_H__
#define __BAO_H__

#include <arch/bao.h>

#ifndef __ASSEMBLER__

#include <types.h>
#include <console.h>
#include <util.h>

/**
 * Console log levels (CONFIG_LOG_LEVEL). A message above the configured level compiles to dead
 * code: its arguments stay visible to the compiler so nothing becomes unused, while the string and
 * the call vanish from the image. ERROR halts the cpu at every level.
 */
#define LOG_LEVEL_NONE    (0)
#define LOG_LEVEL_ERROR   (1)
#define LOG_LEVEL_WARNING (2)
#define LOG_LEVEL_INFO    (3)

#define LOG_DISCARD(...)                 \
    do {                                 \
        if (false) {                     \
            console_printk(__VA_ARGS__); \
        }                                \
    } while (false);

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_INFO
#define INFO(...) console_printk("BAO INFO: " __VA_ARGS__);
#else
#define INFO(...) LOG_DISCARD(__VA_ARGS__)
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_WARNING
#define WARNING(...) console_printk("BAO WARNING: " __VA_ARGS__);
#else
#define WARNING(...) LOG_DISCARD(__VA_ARGS__)
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_ERROR
#define ERROR(...)                             \
    console_printk("BAO ERROR: " __VA_ARGS__); \
    while (true) { };
#else
#define ERROR(...)           \
    LOG_DISCARD(__VA_ARGS__) \
    while (true) { };
#endif

void init(cpuid_t cpu_id);

#endif /* __ASSEMBLER__ */

#endif /* __BAO_H__ */
