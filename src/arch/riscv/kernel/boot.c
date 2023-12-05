/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 * Copyright 2015, 2016 Hesham Almatary <heshamelmatary@gmail.com>
 * Copyright 2021, HENSOLDT Cyber
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <assert.h>
#include <kernel/boot.h>
#include <machine/io.h>
#include <model/statedata.h>
#include <object/interrupt.h>
#include <arch/machine.h>
#include <arch/kernel/boot.h>
#include <arch/kernel/vspace.h>
#include <arch/benchmark.h>
#include <linker.h>
#include <plat/machine/hardware.h>
#include <machine.h>

#ifdef ENABLE_SMP_SUPPORT
BOOT_BSS static volatile word_t node_boot_lock;
#endif
#define UNUSED __attribute__((unused))

extern irq_t active_irq[CONFIG_MAX_NUM_NODES];


/* ASM symbol for the CPU initialisation trap. */
extern char trap_entry[1];


bool_t rust_try_init_kernel(
    paddr_t ui_p_reg_start,
    paddr_t ui_p_reg_end,
    sword_t pv_offset,
    vptr_t v_entry,
    paddr_t dtb_phys_addr,
    word_t dtb_size);

bool_t rust_try_init_kernel_secondary_core(word_t hart_id, word_t core_id);


extern irq_state_t intStateIRQTable[];
extern char kernel_stack_alloc[CONFIG_MAX_NUM_NODES][BIT(CONFIG_KERNEL_STACK_BITS)];
void pRegsToR(word_t *, word_t);
void intStateIRQNodeToR(word_t*);
BOOT_CODE VISIBLE void init_kernel(
    paddr_t ui_p_reg_start,
    paddr_t ui_p_reg_end,
    sword_t pv_offset,
    vptr_t v_entry,
    paddr_t dtb_addr_p,
    uint32_t dtb_size
#ifdef ENABLE_SMP_SUPPORT
    ,
    word_t hart_id,
    word_t core_id
#endif
)
{
    #ifdef CONFIG_DEBUG_BUILD
    printf("CONFIG_DEBUG_BUILD\n");
    #endif
    bool_t result;
    pRegsToR((word_t *)avail_p_regs, ARRAY_SIZE(avail_p_regs));
    intStateIRQNodeToR((word_t*)intStateIRQNode);
#ifdef ENABLE_SMP_SUPPORT
    add_hart_to_core_map(hart_id, core_id);
    if (core_id == 0)
    {
        printf("[c]: init_kernel frist\n");
        result = rust_try_init_kernel(ui_p_reg_start,
                                 ui_p_reg_end,
                                 pv_offset,
                                 v_entry,
                                 dtb_addr_p,
                                 dtb_size);
    }
    else
    {
        // result = try_init_kernel_secondary_core(hart_id, core_id);
        result = rust_try_init_kernel_secondary_core(hart_id, core_id);
    }
#else
    result = rust_try_init_kernel(ui_p_reg_start,
                                  ui_p_reg_end,
                                  pv_offset,
                                  v_entry,
                                  dtb_addr_p,
                                  dtb_size);

#endif
    if (!result)
    {
        fail("ERROR: kernel init failed");
        UNREACHABLE();
    }

#ifdef CONFIG_KERNEL_MCS
    NODE_STATE(ksCurTime) = getCurrentTime();
    NODE_STATE(ksConsumed) = 0;
#endif
    schedule();
    activateThread();
}
