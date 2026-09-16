/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <bao.h>

void console_init(void);
void console_write(const char* buf, size_t n);
void console_printk(const char* fmt, ...);
/* Prints prefix, "func:line: " and the formatted message as one console transaction */
void console_log(const char* prefix, const char* func, int line, const char* fmt, ...);

#endif /* __CONSOLE_H__ */
