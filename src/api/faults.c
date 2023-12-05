/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <types.h>
#include <api/faults.h>
#include <api/syscall.h>
#include <kernel/thread.h>
#include <arch/kernel/thread.h>
#include <machine/debug.h>
#ifdef CONFIG_KERNEL_MCS
#include <mode/api/ipc_buffer.h>
#include <object/schedcontext.h>
#endif

/* consistency with libsel4 */
compile_assert(InvalidRoot, lookup_fault_invalid_root + 1 == seL4_InvalidRoot)
    compile_assert(MissingCapability, lookup_fault_missing_capability + 1 == seL4_MissingCapability)
        compile_assert(DepthMismatch, lookup_fault_depth_mismatch + 1 == seL4_DepthMismatch)
            compile_assert(GuardMismatch, lookup_fault_guard_mismatch + 1 == seL4_GuardMismatch)
                compile_assert(seL4_UnknownSyscall_Syscall, (word_t)n_syscallMessage == seL4_UnknownSyscall_Syscall)
                    compile_assert(seL4_UserException_Number, (word_t)n_exceptionMessage == seL4_UserException_Number)
                        compile_assert(seL4_UserException_Code, (word_t)n_exceptionMessage + 1 == seL4_UserException_Code)


