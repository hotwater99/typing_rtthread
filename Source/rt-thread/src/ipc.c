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