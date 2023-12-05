/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 * Copyright 2015, 2016 Hesham Almatary <heshamelmatary@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <arch/sbi.h>

extern void idle_thread(void);

/** DONT_TRANSLATE */
extern void VISIBLE NO_INLINE halt(void);

// string.c
extern word_t strnlen(const char *s, word_t maxlen);

