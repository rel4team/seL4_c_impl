/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <config.h>
#include <object.h>
#include <util.h>
#include <api/faults.h>
#include <api/types.h>
#include <kernel/cspace.h>
#include <kernel/thread.h>
#include <kernel/vspace.h>
#ifdef CONFIG_KERNEL_MCS
#include <object/schedcontext.h>
#endif
#include <model/statedata.h>
#include <arch/machine.h>
#include <arch/kernel/thread.h>
#include <machine/registerset.h>
#include <linker.h>

seL4_MessageInfo_t
transferCaps(seL4_MessageInfo_t info,
             endpoint_t *endpoint, tcb_t *receiver,
             word_t *receiveBuffer);



#ifdef CONFIG_KERNEL_MCS
static void switchSchedContext(void)
{
    if (unlikely(NODE_STATE(ksCurSC) != NODE_STATE(ksCurThread)->tcbSchedContext))
    {
        NODE_STATE(ksReprogram) = true;
        if (sc_constant_bandwidth(NODE_STATE(ksCurThread)->tcbSchedContext))
        {
            refill_unblock_check(NODE_STATE(ksCurThread)->tcbSchedContext);
        }

        assert(refill_ready(NODE_STATE(ksCurThread)->tcbSchedContext));
        assert(refill_sufficient(NODE_STATE(ksCurThread)->tcbSchedContext, 0));
    }

    if (NODE_STATE(ksReprogram))
    {
        /* if we are reprogamming, we have acted on the new kernel time and cannot
         * rollback -> charge the current thread */
        commitTime();
    }

    NODE_STATE(ksCurSC) = NODE_STATE(ksCurThread)->tcbSchedContext;
}
#endif
void scheduleChooseNewThread(void);

#ifdef CONFIG_KERNEL_MCS
void postpone(sched_context_t *sc)
{
    tcbSchedDequeue(sc->scTcb);
    tcbReleaseEnqueue(sc->scTcb);
    NODE_STATE_ON_CORE(ksReprogram, sc->scCore) = true;
}

void setNextInterrupt(void)
{
    time_t next_interrupt = NODE_STATE(ksCurTime) +
                            refill_head(NODE_STATE(ksCurThread)->tcbSchedContext)->rAmount;

    if (numDomains > 1)
    {
        next_interrupt = MIN(next_interrupt, NODE_STATE(ksCurTime) + ksDomainTime);
    }

    if (NODE_STATE(ksReleaseHead) != NULL)
    {
        next_interrupt = MIN(refill_head(NODE_STATE(ksReleaseHead)->tcbSchedContext)->rTime, next_interrupt);
    }

    setDeadline(next_interrupt - getTimerPrecision());
}

void chargeBudget(ticks_t consumed, bool_t canTimeoutFault)
{
    if (likely(NODE_STATE(ksCurSC) != NODE_STATE(ksIdleSC)))
    {
        if (isRoundRobin(NODE_STATE(ksCurSC)))
        {
            assert(refill_size(NODE_STATE(ksCurSC)) == MIN_REFILLS);
            refill_head(NODE_STATE(ksCurSC))->rAmount += refill_tail(NODE_STATE(ksCurSC))->rAmount;
            refill_tail(NODE_STATE(ksCurSC))->rAmount = 0;
        }
        else
        {
            refill_budget_check(consumed);
        }

        assert(refill_head(NODE_STATE(ksCurSC))->rAmount >= MIN_BUDGET);
        NODE_STATE(ksCurSC)->scConsumed += consumed;
    }
    NODE_STATE(ksConsumed) = 0;
    if (likely(isSchedulable(NODE_STATE(ksCurThread))))
    {
        assert(NODE_STATE(ksCurThread)->tcbSchedContext == NODE_STATE(ksCurSC));
        endTimeslice(canTimeoutFault);
        rescheduleRequired();
        NODE_STATE(ksReprogram) = true;
    }
}

void endTimeslice(bool_t can_timeout_fault)
{
    if (can_timeout_fault && !isRoundRobin(NODE_STATE(ksCurSC)) && validTimeoutHandler(NODE_STATE(ksCurThread)))
    {
        current_fault = seL4_Fault_Timeout_new(NODE_STATE(ksCurSC)->scBadge);
        handleTimeout(NODE_STATE(ksCurThread));
    }
    else if (refill_ready(NODE_STATE(ksCurSC)) && refill_sufficient(NODE_STATE(ksCurSC), 0))
    {
        /* apply round robin */
        assert(refill_sufficient(NODE_STATE(ksCurSC), 0));
        assert(!thread_state_get_tcbQueued(NODE_STATE(ksCurThread)->tcbState));
        SCHED_APPEND_CURRENT_TCB;
    }
    else
    {
        /* postpone until ready */
        postpone(NODE_STATE(ksCurSC));
    }
}
#else


#endif



#ifdef CONFIG_KERNEL_MCS
void awaken(void)
{
    while (unlikely(NODE_STATE(ksReleaseHead) != NULL && refill_ready(NODE_STATE(ksReleaseHead)->tcbSchedContext)))
    {
        tcb_t *awakened = tcbReleaseDequeue();
        /* the currently running thread cannot have just woken up */
        assert(awakened != NODE_STATE(ksCurThread));
        /* round robin threads should not be in the release queue */
        assert(!isRoundRobin(awakened->tcbSchedContext));
        /* threads should wake up on the correct core */
        SMP_COND_STATEMENT(assert(awakened->tcbAffinity == getCurrentCPUIndex()));
        /* threads HEAD refill should always be >= MIN_BUDGET */
        assert(refill_sufficient(awakened->tcbSchedContext, 0));
        possibleSwitchTo(awakened);
        /* changed head of release queue -> need to reprogram */
        NODE_STATE(ksReprogram) = true;
    }
}
#endif
