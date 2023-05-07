/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <types.h>
#include <api/failures.h>

extern lookup_fault_t current_lookup_fault;
extern seL4_Fault_t current_fault;
extern syscall_error_t current_syscall_error;
#ifdef CONFIG_KERNEL_INVOCATION_REPORT_ERROR_IPC
debug_syscall_error_t current_debug_error;
#endif

