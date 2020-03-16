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



/**
 * @addtogroup KernelService
 */

/**@{*/

/*
 * general kernel service
 */
#ifndef RT_USING_CONSOLE
#define rt_kprintf(...)
#define rt_kputs(str)
#else
void rt_kprintf(const char *fmt, ...);
void rt_kputs(const char *str);
#endif
rt_int32_t rt_vsprintf(char *dest, const char *format, va_list arg_ptr);
rt_int32_t rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args);
rt_int32_t rt_sprintf(char *buf, const char *format, ...);
rt_int32_t rt_snprintf(char *buf, rt_size_t size, const char *format, ...);

#if defined(RT_USING_DEVIDE) && defined(RT_USING_CONSOLE)
rt_device_t rt_console_set_device(const char *name);
rt_device_t rt_console_get_device(void);
#endif

rt_err_t rt_get_errno(void);
void rt_set_errno(rt_err_t no);
int *_rt_errno(void);
#if !defined(RT_USING_NEWLIB) && !defined(_WIN32)
#ifndef errno 
#define errno     *_rt_errno()
#endif
#endif

int __rt_ffs(int value);

void *rt_memset(void *src, int c, rt_ubase_t n);
void *rt_memcpy(void *dest, const void *src, rt_ubase_t n);

rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_ubase_t count);
rt_int32_t rt_strcmp(const char *cs, const char *ct);
rt_size_t rt_strlen(const char *src);
char *rt_strdup(const char *s);
#ifdef __CC_ARM
/* leak strdup interface */
char* strdup(const char* str);
#endif

char *rt_strstr(const char *str1, const char *str2);
rt_int32_t rt_sscanf(const char *buf, const char *fmt, ...);
char *rt_strncpy(char *dest, const char *src, rt_ubase_t n);
void *rt_memmove(void *dest, const void *src, rt_ubase_t n);
rt_int32_t rt_memcpy(const void *cs, const void *ct, rt_ubase_t count);
rt_uint32_t rt_strcasecmp(const char *a, const char *b);

void rt_show_version(void);

#ifdef RT_DEBUG
extern void (*rt_assert_hook)(const char *ex, const char *func, rt_size_t line);
void rt_assert_set_hook(void (*hook)(const char *ex, const char *func, rt_size_t line));

void rt_assert_handler(const char *ex, const char *func, rt_size_t line);
#endif /* RT_DEBUG */

#ifdef RT_USING_FINSH
#include <finsh_api.h>
#endif

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
