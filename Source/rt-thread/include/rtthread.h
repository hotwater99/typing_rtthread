#ifndef __RT_THREAD_H__
#define __RT_THREAD_H__

#include <rtconfig.h>
#include <rtdebug.h>
#include <rtdef.h>
#include <rtservice.h>
#include <rtm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup KernelObject
 */

/**@{*/

/*
 * kernel object interface
 */






/**@}*/

/**
 * @addtogroup Clock
 */

/**@{*/

/*
 * clock & timer interface
 */






/**@}*/

/**
 * @addtogroup Thread
 */

/**@{*/

/*
 * thread interface
 */
rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick);
rt_err_t rt_thread_detach(rt_thread_t thread);
rt_thread_t rt_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick);
rt_thread_t rt_thread_self(void);
rt_thread_t rt_thread_find(char *name);

rt_err_t rt_thread_startup(rt_thread_t thread);
rt_err_t rt_thread_delete(rt_thread_t thread);

rt_err_t rt_thread_yield(rt_thread_t thread);
rt_err_t rt_thread_delay(rt_tick_t tick);
rt_err_t rt_thread_mdelay(rt_uint32_t ms);
rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg);
rt_err_t rt_thread_suspend(rt_thread_t thread);
rt_err_t rt_thread_resume(rt_thread_t thread);
void rt_thread_timeout(void *parameter);

#ifdef RT_USING_SIGNALS
void rt_thread_alloc_sig(rt_thread_t tid);
void rt_thread_free_sig(rt_thread_t tid);
void rt_thread_kill(rt_thread_t tid, int sig);
#endif

#ifdef RT_USING_HOOK
void rt_thread_suspend_sethook(void (*hook)(rt_thread_t thread));
void rt_thread_resume_sethook (void (*hook)(rt_thread_t thread));
void rt_thread_init_sethook   (void (*hook)(rt_thread_t thread));
#endif





#ifdef RT_USING_DEVICE
/**
 * @addtogroup Device
 */

/**@{*/

/*
 * device (I/O) system interface
 */

#endif



#ifdef __cplusplus
}
#endif

#endif
