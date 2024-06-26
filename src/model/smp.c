/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <model/smp.h>
#include <object/tcb.h>

#ifdef ENABLE_SMP_SUPPORT

void migrateTCB(tcb_t *tcb, word_t new_core)
{
#ifdef CONFIG_DEBUG_BUILD
    tcbDebugRemove(tcb);
#endif
    Arch_migrateTCB(tcb);
    tcb->tcbAffinity = new_core;
#ifdef CONFIG_DEBUG_BUILD
    tcbDebugAppend(tcb);
#endif
}

cpu_id_t getCurrentCPUIndex(void)
{
    word_t sp;
    asm volatile("csrr %0, sscratch" : "=r"(sp));
    sp -= (word_t)kernel_stack_alloc;
    sp -= 8;
    return (sp >> CONFIG_KERNEL_STACK_BITS);
}


#endif /* ENABLE_SMP_SUPPORT */
