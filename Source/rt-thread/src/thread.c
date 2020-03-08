#include <rtthread.h>
#include <rthw.h>

extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];
extern struct rt_thread *rt_current_thread;
extern rt_list_t rt_thread_defunct;

#ifdef RT_USING_HOOK

static void (*rt_thread_suspend_hook)(rt_thread_t thread);
static void (*rt_thread_resume_hook) (rt_thread_t thread);
static void (*rt_thread_inited_hook) (rt_thread_t thread);

/**
 * @ingroup Hook
 * This function sets a hook function when the system suspend a thread.
 *
 * @param hook the specified hook function
 *
 * @note the hook function must be simple and never be blocked or suspend.
 */
void rt_thread_suspend_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_suspend_hook = hook;
}

/**
 * @ingroup Hook
 * This function sets a hook function when the system resume a thread.
 *
 * @param hook the specified hook function
 *
 * @note the hook function must be simple and never be blocked or suspend.
 */
void rt_thread_resume_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_resume_hook = hook;
}

/**
 * @ingroup Hook
 * This function sets a hook function when a thread is initialized.
 *
 * @param hook the specified hook function
 */
void rt_thread_inited_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_inited_hook = hook;
}

#endif

void rt_thread_exit(void)
{
    struct rt_thread *thread;
    register rt_base_t level;

    /* get current thread */
    thread = rt_current_thread;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* remove from schedule */
    rt_schedule_remove_thread(thread);

    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* remove it from timer list */
    rt_timer_detach(&thread->thread_timer);

    if ((rt_object_is_systemobject((rt_object_t)thread) == RT_TRUE) &&
        thread->cleanup == RT_NULL)
    {
        rt_object_detach((rt_object_t)thread);
    }
    else
    {
        /* insert to defunct thread list */
        rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    /* switch to next task */
    rt_schedule();
}

static rt_err_t _rt_thread_init(struct rt_thread *thread,
                                const char       *name,
                                void (*entry)(void *parameter),
                                void             *parameter,
                                void             *stack_start,
                                rt_uint32_t       stack_size,
                                rt_uint8_t        priority,
                                rt_uint32_t       tick)
{
    /* init thread list */
    rt_list_init(&(thread->tlist));

    thread->entry = (void *)entry;
    thread->parameter = parameter;

    /* stack init */
    thread->stack_addr = stack_start;
    thread->stack_size = stack_size;

    /* init thread stack */
    rt_memset(thread->stack_addr, '#', thread->stack_size);
    thread->sp = (void *)rt_hw_stack_init(thread->entry, 
                                          thread->parameter,
                                          (void *)((char *)thread->stack_addr + thread->stack_size - 4),
                                          (void *)rt_thread_exit);

    /* priority init */
    RT_ASSERT(priority < RT_THREAD_PRIORITY_MAX);
    thread->init_priority    = priority;
    thread->current_priority = priority;

    thread->number_mask = 0;
#if RT_THREAD_PRIORITY_MAX > 32
    thread->number      = 0;
    thread->high_mask   = 0;
#endif

    /* tick init */
    thread->init_tick      = tick;
    thread->remaining_tick = tick;

    /* error and flags */
    thread->error = RT_EOK;
    thread->stat  = RT_THREAD_INIT;

    /* initialize cleanup function and user data */
    thread->cleanup   = 0;
    thread->user_data = 0;

    /* init thread timer */
    rt_timer_init(&(thread->thread_timer),
                  thread->name,
                  rt_thread_timeout,
                  thread,
                  0,
                  RT_TIMER_FLAG_ONE_SHOT);

    /* initialize signal */
#ifdef RT_USING_SIGNALS
    thread->sig_mask    = 0x00;
    thread->sig_pending = 0x00;

    thread->sig_ret     = RT_NULL;
    thread->sig_vectors = RT_NULL;
    thread->si_list     = RT_NULL;
#endif

#ifdef RT_USING_LWP
    thread->lwp = RT_NULL；
#endif

    RT_OBJECT_HOOK_CALL(rt_thread_inited_hook, (thread));

    return RT_EOK;
}

/**
 * @addtogroup Thread
 */

/**@{*/

/**
 * This function will initialize a thread, normally it's used to initialize a
 * static thread object.
 *
 * @param thread the static thread object
 * @param name the name of thread, which shall be unique
 * @param entry the entry function of thread
 * @param parameter the parameter of thread enter function
 * @param stack_start the start address of thread stack
 * @param stack_size the size of thread stack
 * @param priority the priority of thread
 * @param tick the time slice if there are same priority thread
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint32_t       priority,
                        rt_uint32_t       tick)
{
    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT(stack_start != RT_NULL);

    /* init thread object */
    rt_object_init((rt_object_t)thread, RT_Object_Class_Thread, name);

    return _rt_thread_init(thread,
                           name,
                           entry,
                           parameter,
                           stack_start,
                           stack_size,
                           priority,
                           tick);
}
RTM_EXPORT(rt_thread_init);

/**
 * This function will return self thread object
 *
 * @return the self thread object
 */
rt_thread_t rt_thread_self(void)
{
    return rt_current_thread;
}
RTM_EXPORT(rt_thread_self);

/**
 * This function will start a thread and put it to system ready queue
 *
 * @param thread the thread to be started
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_startup(rt_thread_t thread)
{
    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_INIT);
    RT_ASSERT(rt_object_get_type((rt_object_t)thread) == RT_Object_Class_Thread);

    /* set current priority to init priority */
    thread->current_priority = thread->init_priority;

    /* calculate priority attribute */
#if RT_THREAD_PRIORITY_MAX > 32
    thread->number      = thread->current_priority >> 3;            /* 5bit */
    thread->number_mask = 1L << thread->number;
    thread->high_mask   = 1L << (thread->current_priority & 0x07);  /* 3bit */
#else
    thread->number_mask = 1L << thread->current_priority;
#endif

    RT_DEBUG_LOG(RT_DEBUG_THREAD, ("startup a thread:%s with priority:%d\n",
                                   thread->name, thread->init_priority));

    /* change thread stat */
    thread->stat = RT_THREAD_SUSPEND;
    /* then resume it */
    rt_thread_resume(thread);
    if (rt_thread_self() != RT_NULL)
    {
        /* do a scheduling */
        rt_schedule();
    }
    
    return RT_EOK；
}
RTM_EXPORT(rt_thread_startup);

/**
 * This function will detach a thread. The thread object will be removed from
 * thread queue and detached/deleted from system object management.
 *
 * @param thread the thread to be deleted
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_detach(rt_thread_t thread)
{
    rt_base_t lock;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT(rt_object_get_type((rt_object_t)thread) == RT_Object_Class_Thread);
    RT_ASSERT(rt_object_is_systemobject((rt_object_t)thread));

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_INIT)
    {
        /* remove from schedule */
        rt_schedule_remove_thread(thread);
    }

    /* release thread timer */
    rt_timer_detach(&(thread->thread_timer));

    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* detach object */
    rt_object_detach((rt_object_t)thread);

    if (thread->cleanup != RT_NULL)
    {
        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        /* insert to defunct thread list */
        rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));

        /* enable interrupt */
        rt_hw_interrupt_enable(lock);
    }

    return RT_EOK;
}
RTM_EXPORT(rt_thread_detach);

#ifdef RT_USING_HEAP
/**
 * This function will create a thread object and allocate thread object memory
 * and stack.
 *
 * @param name the name of thread, which shall be unique
 * @param entry the entry function of thread
 * @param parameter the parameter of thread enter function
 * @param stack_size the size of thread stack
 * @param priority the priority of thread
 * @param tick the time slice if there are same priority thread
 *
 * @return the created thread object
 */
rt_thread_t rt_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick)
{
    struct rt_thread *thread;
    void *stack_start;

    thread = (struct rt_thread *)rt_object_allocate(RT_Object_Class_Thread, 
                                                    name);
    if(thread == RT_NULL)
        return RT_NULL;

    stack_start = (void *)RT_KERNAL_MALLOC(stack_size);
    if (stack_start == RT_NULL)
    {
        /* allocate stack failure */
        rt_object_delete((rt_object_t)thread);

        return RT_NULL;
    }

    _rt_thread_init(thread,
                    name,
                    entry,
                    parameter,
                    stack_start,
                    stack_size,
                    priority,
                    tick);

    return thread;
}
RTM_EXPORT(rt_thread_create);

/**
 * This function will delete a thread. The thread object will be removed from
 * thread queue and deleted from system object management in the idle thread.
 *
 * @param thread the thread to be deleted
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_delete(rt_thread_t thread)
{
    rt_base_t lock;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT(rt_object_get_type((rt_object_t)thread) == RT_Object_Class_Thread);
    RT_ASSERT(rt_object_is_systemobject((rt_object_t)thread) == RT_FALSE);

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_INIT)
    {
        /* remove from schedule */
        rt_schedule_remove_thread(thread);
    }

    /* release thread timer */
    rt_timer_detach(&(thread->thread_timer));

    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* disable interrupt */
    lock = rt_hw_interrupt_disable();

    /* insert to defunct thread list */
    rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));

    /* enable interrupt */
    rt_hw_interrupt_enable(lock);

    return RT_EOK;
}
RTM_EXPORT(rt_thread_delete);
#endif

/**
 * This function will let current thread yield processor, and scheduler will
 * choose a highest thread to run. After yield processor, the current thread
 * is still in READY state.
 *
 * @return RT_EOK
 */
rt_err_t rt_threead_yield(void)
{
    register rt_base_t level;
    struct rt_thread *thread;

    /* disable interrupt */
    levle = rt_hw_interrupt_disable();

    /* set to current thread */
    thread = rt_current_thread;

    /* if the thread stat is READY and on ready queue list */
    if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_READY &&
        thread->tlist.next != thread->tlist.prev)
    {
        /* remove thread from thread list */
        rt_list_remove(&(thread->tlist));

        /* put thread to end of ready queue */
        rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                              &(thread->tlist));

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);
    
    return RT_EOK;
}
RTM_EXPORT(rt_thread_yield);

/**
 * This function will let current thread sleep for some ticks.
 *
 * @param tick the sleep ticks
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_sleep(rt_tick_t tick)
{
    /* disable interrupt */
    /* set to current thread */
    /* suspend thread */
    /* reset the timeout of thread timer and start it */
    /* enable interrupt */
    /* clear error number of this thread to RT_EOK */
    
    return RT_EOK;
}