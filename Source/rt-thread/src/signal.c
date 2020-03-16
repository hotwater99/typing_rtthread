#include <stdint.h>
#include <string.h>

#include <rthw.h>
#include <rtthread.h>

#ifdef RT_USING_SIGNALS

#ifndef RT_SIG_INFO_MAX
#define RT_SIG_INFO_MAX 32
#endif

// #define DBG_ENABLE
#define DBG_SECTION_NAME    "SIGN"
#define DBG_COLOR
#define DBG_LEVEL           DBG_LOG
#include <rtdbg.h>

#define sig_mask(sig_no)    (1u << sig_no)
#define sig_valid(sig_no)   (sig_no > 0 && sig_no < RT_SIG_MAX)

struct siginfo_node
{
    siginfo_t si;
    struct rt_slist_node list;
};

static struct rt_mempool *_rt_siginfo_pool;
static void _signal_deliver(rt_thread_t tid);
void rt_thread_handle_sig(rt_bool_t clean_state);

static void _signal_default_handler(int signo)
{
    dbg_log(DBG_INFO, "handled signo[%d] with default action.\n", signo);
    return ;
}

static void _signal_entry(void *parameter)
{
    rt_thread_t tid = rt_thread_self();

    dbg_enter;

    /* handle signal */
    rt_thread_handle_sig(RT_FALSE);

    /* never come back... */
    rt_hw_interrupt_disable();
    /* return to thread */
    tid->sp = tid->sig_ret;
    tid->sig_ret = RT_NULL;

    dbg_log(DBG_LOG, "switch back to: 0x0%08x\n", tid->sp);
    tid->stat &= ~RT_THREAD_STAT_SIGNAL;

    rt_hw_context_switch_to((rt_uint32_t) & (tid->sp));
}

/*
 * To deliver a signal to thread, there are cases:
 * 1. When thread is suspended, function resumes thread and
 * set signal stat;
 * 2. When thread is ready:
 *   - If function delivers a signal to self thread, just handle
 *    it.
 *   - If function delivers a signal to another ready thread, OS
 *    should build a slice context to handle it.
 */
static void _signal_deliver(rt_thread_t tid)
{
    rt_ubase_t level;

    /* thread is not interested in pended signals */
    if (!(tid->sig_pending & tid->sig_mask)) return;

    level = rt_hw_interrupt_disable();
    if ((tid->stat & RT_THREAD_STAT_MASK) == RT_THREAD_SUSPEND)
    {
        /* resume thread to handle signal */
        rt_thread_resume(tid);
        /* add signal state */
        tid->stat |= RT_THREAD_STAT_SIGNAL;

        rt_hw_interrupt_enable(level);

        /* re-schedule */
        rt_schedule();
    }
    else
    {
        if (tid == rt_thread_self())
        {
            /* add signal state */
            tid->stat |= RT_THREAD_STAT_SIGNAL;

            rt_hw_interrupt_enable(level);

            /* do signal action in self thread context */
            rt_thread_handle_sig(RT_TRUE);
        }
        else if (!((tid->stat & RT_THREAD_STAT_MASK) & RT_THREAD_STAT_SIGNAL))
        {
            /* add signal state */
            tid->stat |= RT_THREAD_STAT_SIGNAL;

            /* point to the signal handle entry */
            tid->sig_ret = tid->sp;
            tid->sp = rt_hw_stack_init((void *)_signal_entry, RT_NULL,
                                       (void *)((char *)tie->sig_ret - 32), RT_NULL);

            rt_hw_interrupt_enable(level);
            dbg_log(DBG_LOG, "signal stack pointer @ 0x%08x\n", tid->sp);

            /* re-schedule */
            rt_schedule();
        }
        else
        {
            rt_hw_interrupt_enable(level);
        }
    }
}

rt_sighandler_t rt_signal_install(int signo, rt_sighandler_t handler)
{
    rt_sighandler_t old = RT_NULL;
    rt_thread_t tid = rt_thread_self();

    if (!sig_valid(signo)) return SIG_ERR;

    rt_enter_critical();
    if (tid->sig_vectors == RT_NULL)
    {
        rt_thread_alloc_sig(tid);
    }

    if (tid->sig_vectors)
    {
        old = tid->sig_vectors[signo];

        if (handler == SIG_IGN) tid->sig_vectors[signo] = RT_NULL;
        else if (handler == SIG__DFL) tid->sig_vectors[signo] = _sig_default_handler;
        else tid->sig_vecotors[signo] = handler;
    }
    rt_exit_critical();

    return old;
}

void rt_signal_mask(int signo)
{
    rt_base_t level;
    rt_thread_t tid = rt_thread_self();

    level = rt_hw_interrupt_disable();

    tid->sig_mask &= ~sig_mask(signo);

    rt_hw_interrupt_enable(level);
}

void rt_signal_unmask(int signo)
{
    rt_base_t level;
    rt_thread_t tid = rt_thread_self();

    level = rt_hw_interrupt_disable();

    tid->sig_mask |= sig_mask(signo);

    /* let thread handle pended signals */
    if (tid->sig_mask & tid->sig_pending)
    {
        rt_hw_interrupt_enable(level);
        _signal_diliver(tid);
    }
    else
    {
        rt_hw_interrupt_enable(level);
    }
}

#endif