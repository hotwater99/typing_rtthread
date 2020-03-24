#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_HOOK
extern void (*rt_object_trytake_hook)(struct rt_object *object);
extern void (*rt_object_take_hook)(struct rt_object *object);
extern void (*rt_object_put_hook)(struct rt_object *object);
#endif

/**
 * @addtogroup IPC
 */

/**@{*/

/**
 * This function will initialize an IPC object
 *
 * @param ipc the IPC object
 *
 * @return the operation status, RT_EOK on successful
 */
rt_inline rt_err_t rt_ipc_object_init(struct rt_ipc_object *ipc)
{
    /* init ipc object */
    rt_list_init(&(ipc->suspend_thread));

    return RT_EOK;
}

/**
 * This function will suspend a thread to a specified list. IPC object or some
 * double-queue object (mailbox etc.) contains this kind of list.
 *
 * @param list the IPC suspended thread list
 * @param thread the thread object to be suspended
 * @param flag the IPC object flag,
 *        which shall be RT_IPC_FLAG_FIFO/RT_IPC_FLAG_PRIO.
 *
 * @return the operation status, RT_EOK on successful
 */
rt_inline rt_err_t rt_ipc_list_suspend(rt_list_t        *list,
                                       struct rt_thread *thread,
                                       rt_uint8_t        flag)
{
    /* suspend thread */
    rt_thread_suspend(thread);

    switch (flag)
    {
    case RT_IPC_FLAG_FIFO:
        rt_list_insert_before(list, &(thread->tlist));
        break;

    case RT_IPC_FLAG_PRIORITY:
        struct rt_list_node *n;
        struct rt_thread *sthread;

        /* find a suitable position */
        for (n = list->next; n != list; n = n->next)
        {
            sthread = rt_list_entry(n, struct rt_thread, tlist);

            /* find out */
            if (thread->current_priority < sthread->current_priority)
            {
                /* insert this thread before the sthread */
                rt_list_insert_before(&(sthrad->tlist), &(thread->tlist));
                break;
            }
        }

        /*
         * not found a suitable position,
         * append to the end of suspend_thread list
         */
        if (n == list)
            rt_list_insert_before(list, &(thread->tlist));

        break;
    }

    return RT_EOK;
}

/**
 * This function will resume the first thread in the list of a IPC object:
 * - remove the thread from suspend queue of IPC object
 * - put the thread into system ready queue
 *
 * @param list the thread list
 *
 * @return the operation status, RT_EOK on successful
 */
rt_inline rt_err_t rt_ipc_list_resume(rt_list_t *list)
{
    struct thread *thread;

    /* get thread entry */
    thread = rt_list_entry(list->next, struct rt_thread, tlist);

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("resume thread %s\n", thread->name));

    /* resume it */
    rt_thread_resume(thread);

    return RT_EOK;
}

/**
 * This function will resume all suspended threads in a list, including
 * suspend list of IPC object and private list of mailbox etc.
 *
 * @param list of the threads to resume
 *
 * @return the operation status, RT_EOK on successful
 */
rt_inline rt_err_t rt_ipc_list_resume_all(rt_list_t *list)
{
    struct rt_thread *thread;
    register rt_ubasse_t temp;

    /* wakeup all suspend threads */
    while (!rt_list_isempty(list))
    {
        /* disable interrupt */
        temp = rt_hw_interrupt_disabled();

        /* get next suspend thread */
        thread = rt_list_entry(list->next, struct rt_thread, tlist);
        /* set error code to RT_ERROR */
        thread->error = -RT_ERROR;

        /*
         * resume thread
         * In rt_thread_resume function, it will remove current thread from
         * suspend list
         */
        rt_thread_resume(thread);

        /* enable interrupt */
        rt_hw_interrupt_enabled(temp);
    }
    
    return RT_EOK;
}

#ifdef RT_USING_SEMAPHORE
/**
 * This function will initialize a semaphore and put it under control of
 * resource management.
 *
 * @param sem the semaphore object
 * @param name the name of semaphore
 * @param value the init value of semaphore
 * @param flag the flag of semaphore
 *
 * @return the operation status, RT_EOK on successful
 */
rt_err_t rt_sem_init(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag)
{
    RT_ASSERT(sem != RT_NULL);

    /* init object */
    rt_object_init(&(sem->parent.parent), RT_Object_Class_Semaphore, name);

    /* init ipc object */
    rt_ipc_object_init(&(sem->parent));

    /* set init value */
    sem->value = value;

    /* set parent */
    sem->parent.parent.flag = flag;

    return RT_EOK;
}
RTM_EXPORT(rt_sem_init);

/**
 * This function will detach a semaphore from resource management
 *
 * @param sem the semaphore object
 *
 * @return the operation status, RT_EOK on successful
 *
 * @see rt_sem_delete
 */
rt_err_t rt_sem_detach(rt_sem_t sem)
{
    /* parameter check */
    RT_ASSERT(sem != RT_NULL);
    RT_ASSERT(rt_object_get_type(&(sem->parent.parent)) == RT_Object_Class_Semaphore);
    RT_ASSERT(rt_object_is_systemobject(&(sem->parent.parent)));

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(sem->parent.suspend_thread));

    /* detach semaphore object */
    rt_object_detach(&(sem->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_detach);

#ifdef RT_USING_HEAP
/**
 * This function will create a semaphore from system resource
 *
 * @param name the name of semaphore
 * @param value the init value of semaphore
 * @param flag the flag of semaphore
 *
 * @return the created semaphore, RT_NULL on error happen
 *
 * @see rt_sem_init
 */
rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag)
{
    rt_sem_t sem;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    sem = (rt_sem_t)rt_object_allocate(RT_Object_Class_Semaphore, name);
    if (sem == RT_NULL)
        return sem;

    /* init ipc object */
    rt_ipc_object_init(&(sem->parent));

    /* set init value */
    sem->value = value;

    /* set parent */
    sem->parent.parent.flag = flag;

    return sem;
}
RTM_EXPORT(rt_sem_create);

/**
 * This function will delete a semaphore object and release the memory
 *
 * @param sem the semaphore object
 *
 * @return the error code
 *
 * @see rt_sem_detach
 */
rt_err_t rt_sem_delete(rt_sem_t sem)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    /* parameter check */
    RT_ASSERT(sem != RT_NULL);
    RT_ASSERT(rt_object_get_type(&(sem->parent.parent)) == RT_Object_Class_Semaphore);
    RT_ASSERT(rt_object_is_systemobject(&(sem->parent.parent)) == RT_FALSE);

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(sem->parent.suspend_thread));

    /* delete semaphore object */
    rt_object_delete(&(sem->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_delete);
#endif /* end of RT_USING_HEAP */

/**
 * This function will take a semaphore, if the semaphore is unavailable, the
 * thread shall wait for a specified time.
 *
 * @param sem the semaphore object
 * @param time the waiting time
 *
 * @return the error code
 */
rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time)
{
    register rt_base_t temp;
    struct rt_thread *thread;

    /* parameter check */
    RT_ASSERT(sem != RT_NULL);
    RT_ASSERT(rt_object_get_type(&(sem->parent.parent)) == RT_Object_Class_Semaphore);

    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(sem->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disabled();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s take sem %s, which value is: %d\n",
                                rt_thread_self()->name,
                                ((struct rt_object *)sem)->name,
                                sem->value));

    if (sem->value > 0)
    {
        /* semaphore is available */
        sem->value --;

        /* enable interrupt */
        rt_hw_interrupt_enabled(temp);
    }
    else
    {
        /* no waiting, return with timeout */
        if (time == 0)
        {
            rt_hw_interrupt_enabled(temp);

            return -RT_ETIMEOUT;
        }
        else
        {
            /* current context checking */
            RT_DEBUG_IN_THREAD_CONTEXT;

            /* semaphore is unavailable, push to suspend list */
            /* get current thread */
            thread = rt_thread_self();

            /* reset thread error number */
            thread->error = RT_EOK;
            RT_DEBUG_LOG(RT_DEBUG_IPC, ("sem take: suspend thread - %s\n",
                                        thread->name));

            /* suspend thread */
            rt_ipc_list_suspend(&(sem->parent.suspend_thread),
                                thread,
                                sem->parent.parent.flag);

            /* has waiting time, start thread timer */
            if (time > 0)
            {
                RT_DEBUG_LOG(RT_DEBUG_IPC, ("set thread: %s to timer list\n",
                                            thread->name));

                /* reset the timeout of thread timer and start it */
                rt_timer_control(&(thread->thread_timer),
                                 RT_TIMER_CTRL_SET_TIME,
                                 &time);
                rt_timer_start(&(thread->thread_timer));
            }

            /* enable interrupt */
            rt_hw_interrupt_enabled(temp);

            /* do schedule */
            rt_schedule();

            if (thread->error != RT_EOK)
            {
                return thread->error;
            }
        }
    }

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(sem->parent.parent)));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_take);

/**
 * This function will try to take a semaphore and immediately return
 *
 * @param sem the semaphore object
 *
 * @return the error code
 */
rt_err_t rt_sem_trytake(rt_sem_t sem)
{
    return rt_sem_take(sem, 0);
}
RTM_EXPORT(rt_sem_trytake);

/**
 * This function will release a semaphore, if there are threads suspended on
 * semaphore, it will be waked up.
 *
 * @param sem the semaphore object
 *
 * @return the error code
 */
rt_err_t rt_sem_release(rt_sem_t sem)
{
    register rt_base_t temp;
    register rt_bool_t need_schedule;

    /* parameter check */
    RT_ASSERT(sem != RT_NULL);
    RT_ASSERT(rt_object_get_type(&(sem->parent.parent)) == RT_Object_Class_Semaphore);

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(sem->parent.parent)));

    need_schedule = RT_FALSE;

    /* disable interrupt */
    temp = rt_hw_interrupt_disabled();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s release sem: %s, which value is: %d\n",
                                rt_thread_self()->name,
                                ((struct rt_object *)sem)->name,
                                sem->value));

    if (!rt_list_isempty(&(sem->parent.suspend_thread)))
    {
        /* resume the suspended thread */
        rt_ipc_list_resume(&(sem->parent.suspend_thread));
        need_schedule = RT_TRUE;
    }
    else
    {
        /* increase value */
        sem->value ++;
    }

    /* enable interrupt */
    rt_hw_interrupt_enabled(temp);

    /* resume a thread, re-schedule */
    if (need_schedule == RT_TRUE)
        rt_schedule();

    return RT_EOK;
}
RTM_EXPORT(rt_sem_release);

/**
 * This function can get or set some extra attributions of a semaphore object.
 *
 * @param sem the semaphore object
 * @param cmd the execution command
 * @param arg the execution argument
 *
 * @return the error code
 */
rt_err_t rt_sem_control(rt_sem_t sem, int cmd, void *arg)
{
    rt_ubase_t level;

    /* parameter check */
    RT_ASSERT(sem != RT_NULL);
    RT_ASSERT(rt_object_get_type(&(sem->parent.parent)) == RT_Object_Class_Semaphore);

    if (cmd == RT_IPC_CMD_RESET)
    {
        rt_uint32_t value;

        /* get value */
        value = (rt_uint32_t)arg;

        /* disable interrupt */
        level = rt_hw_interrupt_disabled();

        /* resume all waiting thread */
        rt_ipc_list_resume_all(&sem->parent.suspend_thread);

        /* set new value */
        sem->value = (rt_uint16_t)value;

        /* enable interrupt */
        rt_hw_interrupt_enabled(level);

        rt_schedule();

        return RT_EOK;
    }

    return -RT_ERROR;
}
RTM_EXPORT(rt_sem_control);
#endif /* end of RT_USING_SEMAPHORE */













