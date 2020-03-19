#ifndef __RT_DEF_H__
#define __RT_DEF_H__

/* include rtconfig header to import configuration */
#include <rtconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup BasicDef
 */

/*@{*/

/* RT-Thread version information */
#define RT_VERSION                      3L              /**< major version number */
#define RT_SUBVERSION                   1L              /**< minor version number */
#define RT_REVISION                     0L              /**< revise version number */

/* RT-Thread version */
#define RTTHREAD_VERSION                ((RT_VERSION * 10000) + \
                                         (RT_SUBVERSION * 100) + RT_REVISION)

/* RT-Thread basic data type definitions */
typedef signed   char                   rt_int8_t;
typedef signed   short                  rt_int16_t;
typedef signed   long                   rt_int32_t;
typedef unsigned char                   rt_uint8_t;
typedef unsigned short                  rt_uint16_t;
typedef unsigned long                   rt_uint32_t;
typedef int                             rt_bool_t;

/* 32bit CPU */
typedef long                            rt_base_t;
typedef unsigned long                   rt_ubase_t;

typedef rt_base_t                       rt_err_t;
typedef rt_uint32_t                     rt_time_t;
typedef rt_uint32_t                     rt_tick_t;
typedef rt_base_t                       rt_flag_t;
typedef rt_ubase_t                      rt_size_t;
typedef rt_ubase_t                      rt_dev_t;
typedef rt_base_t                       rt_off_t;

/* boolean type definitions */
#define RT_TRUE                         1
#define RT_FALSE                        0

/*@}*/

/* maximum value of base type */
#define RT_UINT8_MAX                    0xff 
#define RT_UINT16_MAX                   0xffff
#define RT_UINT32_MAX                   0xffffffff
#define RT_TICK_MAX                     RT_UINT32_MAX

/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __weak
    #define rt_inline                   static __inline
    /* module compiling */
    #ifdef RT_USING_MODULE
        #define RTT_API                 __declspec(dllimport)
    #else
        #define RTT_API                 __declspec(dllexport)
    #endif

/* >_< code for other compilers: using copy and paste */
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    
    #include <stdarg.h>
    #define SECTION(x)                  @ x
    #define RT_UNUSED
    #define RT_USED                     __root
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
    #define RT_WEAK                     __weak
    #define rt_inline                   static inline
    #define RTT_API

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #ifdef RT_USING_NEWLIB
        #include <stdarg.h>
    #else
        /* the version of GNU GCC must be greater than 4.x */
        typedef __builtin_va_list   __gnuc_va_list;
        typedef __gnuc_va_list      va_list;
        #define va_start(v,l)       __builtin_va_start(v,l)
        #define va_end(v)           __builtin_va_end(v)
        #define va_arg(v,l)         __builtin_va_arg(v,l)
    #endif

    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static __inline
    #define RTT_API

#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static inline
    #define RTT_API

#elif defined (_MSC_VER)
    #include <stdarg.h>
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define ALIGN(n)                    __declspec(align(n))
    #define RT_WEAK
    #define rt_inline                   static __inline
    #define RTT_API

#elif defined (__TI_COMPILER_VERSION__)
    #include <stdarg.h>
    /* The way that TI compiler set section is different from other(at least
     * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
     * details. */
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)
    #define RT_WEAK
    #define rt_inline                   static inline
    #define RTT_API

#else
    #error not support tool chain
#endif

/* initialization export */
#ifdef RT_USING_COMPONENTS_INIT
typedef int (*init_fn_t)(void);
#ifdef _MSC_VER /* we do not support MS VC++ compiler */
    #define INIT_EXPORT(fn, level)
#else
    #if RT_DEBUG_INIT
        struct rt_init_desc
        {
            const char* fn_name;
            const init_fn_t fn;
        };
        #define INIT_EXPORT(fn, level)                                                       \
            const char __rti_##fn##_name[] = #fn;                                            \
            RT_USED const struct rt_init_desc __rt_init_desc_##fn SECTION(".rti_fn."level) = \
            {__rti_##fn##_name, fn};
    #else
        #define INIT_EXPORT(fn, level)                                                       \
            RT_USED const init_fn_t __rt_init_##fn SECTION(".rti_fn."level) = fn
    #endif
#endif

#else /* RT_USING_COMPONENTS_INIT */
#define INIT_EXPORT(fn, level)
#endif /* RT_USING_COMPONENTS_INIT */

/* board init routines will be called in board_init() function */
#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")

/* pre/device/component/env/app init routines will be called in init_thread */
/* components pre-initialization (pure software initilization) */
#define INIT_PREV_EXPORT(fn)            INIT_EXPORT(fn, "2")

/* device initialization */
#define INIT_DEVICE_EXPORT(fn)          INIT_EXPORT(fn, "3")

/* components initialization (dfs, lwip, ...) */
#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "4")

/* environment initialization (mount disk, ...) */
#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "5")

/* appliation initialization (rtgui application etc ...) */
#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "6")

#if !defined(RT_USING_FINSH)
/* define these to empty, even if not include finsh.h file */
#define FINSH_FUNCTION_EXPORT(name, desc)
#define FINSH_FUCNTION_EXPORT_ALIAS(name, alias, desc)
#define FINSH_VAR_EXPORT(name, type, desc)

#define MSH_CMD_EXPORT(command, desc)
#define MSH_CMD_EXPORT_ALIAS(command, alias, desc)
#elif !defined(FINSH_USING_SYMTAB)
#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)
#endif

/* event length */
#define RT_EVENT_LENGTH                 32

/* memory management option */
#define RT_MM_PAGE_SIZE                 4096
#define RT_MM_PAGE_MASK                 (RT_MM_PAGE_SIZE - 1)
#define RT_MM_PAGE_BITS                 12

/* kernel malloc definitions */
#ifndef RT_KERNEL_MALLOC
#define RT_KERNAL_MALLOC(sz)            rt_malloc(sz)
#endif

#ifndef RT_KERNAL_FREE
#define RT_KERNAL_FREE(ptr)             rt_free(ptr)
#endif

#ifndef RT_KERNAL_REALLOC
#define RT_KERNAL_REALLOC(ptr, size)    rt_realloc(ptr, size)
#endif

/**
 * @addtogroup Error
 */

/*@{*/

/* RT-Thread error code definitions */
#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        1               /**< A generic error happens */
#define RT_ETIMEOUT                     2               /**< Timed out */
#define RT_EFULL                        3               /**< The resource is full */
#define RT_EEMPTY                       4               /**< The resource is empty */
#define RT_ENOMEM                       5               /**< No memory */
#define RT_ENOSYS                       6               /**< No system */
#define RT_EBUSY                        7               /**< Busy */
#define RT_EIO                          8               /**< IO error */
#define RT_EINTR                        9               /**< Interrupted system call */
#define RT_EINVAL                       10              /**< Invalid argument */

/*@}*/

/**
 * @ingroup BasicDef
 *
 * @def RT_ALIGN(size, align)
 * Return the most contiguous size aligned at specified width. RT_ALIGN(13, 4)
 * would return 16.
 */
#define RT_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

/**
 * @ingroup BasicDef
 *
 * @def RT_ALIGN_DOWN(size, align)
 * Return the down number of aligned at specified width. RT_ALIGN_DOWN(13, 4)
 * would return 12.
 */
#define RT_ALIGN_DOWN(size, align)     ((size) & ~((align) - 1))

/**
 * @ingroup BasicDef
 *
 * @def RT_NULL
 * Similar as the \c NULL in C library.
 */  
#define RT_NULL                         (0)

/**
 * Double List structure
 */
struct rt_list_node
{
    struct rt_list_node *next;
    struct rt_list_node *prev;
};
typedef struct rt_list_node rt_list_t;

/**
 * Single List structure
 */
struct rt_slist_node
{
    struct rt_slisst_node *next;
};
typedef struct rt_slist_node rt_slist_t;

/**
 * @addtogroup KernelObject
 */

/*@{*/

/*
 * kernel object macros
 */
#define RT_OBJECT_FLAG_MODULE           0x80            /**< is module object. */

/**
 * Base structure of Kernel object
 */
struct rt_object
{
    char       name[RT_NAME_MAX];                       /**< name of kernel object */
    rt_uint8_t type;                                    /**< type of kernel object */
    rt_uint8_t flag;                                    /**< flag of kernel object */

#ifdef RT_USING_MODULE
    void      *module_id;                               /**< id of application module */
#endif
    rt_list_t  list;                                    /**< list node of kernel object */
};
typedef struct rt_object *rt_object_t;                  /**< Type for kernel objects. */

/**
 *  The object type can be one of the follows with specific
 *  macros enabled:
 *  - Thread
 *  - Semaphore
 *  - Mutex
 *  - Event
 *  - MailBox
 *  - MessageQueue
 *  - MemHeap
 *  - MemPool
 *  - Device
 *  - Timer
 *  - Module
 *  - Unknown
 *  - Static
 */
enum rt_object_class_type
{
    RT_Object_Class_Thread = 0,
    RT_Object_Class_Semaphore,
    RT_Object_Class_Mutex,
    RT_Object_Class_Event,
    RT_Object_Class_MailBox,
    RT_Object_Class_MessageQueue,
    RT_Object_Class_MemHeap,
    RT_Object_Class_MemPool,
    RT_Object_Class_Device,
    RT_Object_Class_Timer,
    RT_Object_Class_Module,
    RT_Object_Class_Unknown,
    RT_Object_Class_Static = 0x80
};

/**
 * The information of the kernel object
 */
struct rt_object_information
{
    enum rt_object_class_type type;
    rt_list_t                 object_list;
    rt_size_t                 object_size;
};

/**
 * The hook function call macro
 */
#ifdef RT_USING_HOOK
#define RT_OBJECT_HOOK_CALL(func, argv) \
    do { if ((func) != RT_NULL) func argv; } while (0)
#else
#define RT_OBJECT_HOOK_CALL(func, argv)
#endif

/*@}*/

/**
 * @addtogroup Clock
 */

/*@{*/

/**
 * clock & timer macros
 */
#define RT_TIMER_FLAG_DEACTIVATED       0x0             /**< timer is deactive */
#define RT_TIMER_FLAG_ACTIVATED         0x1             /**< timer is active */
#define RT_TIMER_FLAG_ONE_SHOT          0x0             /**< one shot timer */
#define RT_TIMER_FLAG_PERIODIC          0x2             /**< periodic timer */

#define RT_TIMER_FLAG_HARD_TIMER        0x0
#define RT_TIMER_FLAG_SOFT_TIMER        0x4

#define RT_TIMER_CTRL_SET_TIME          0x0
#define RT_TIMER_CTRL_GET_TIME          0x1
#define RT_TIMER_CTRL_SET_ONESHOT       0x2
#define RT_TIMER_CTRL_SET_PERIODIC      0x3

#ifndef RT_TIMER_SKIP_LIST_LEVEL
#define RT_TIMER_SKIP_LIST_LEVEL        1
#endif

/* 1 or 3 */
#ifndef RT_TIMER_SKIP_LIST_MASK
#define RT_TIMER_SKIP_LIST_MASK         0x3
#endif

/**
 * timer structure
 */
struct rt_timer
{
    struct rt_object parent;

    rt_list_t        row[RT_TIMER_SKIP_LIST_LEVEL];

    void (*timeout_func)(void *parameter);              /**< timeout function */
    void            *parameter;                         /**< timeout function's parameter */

    rt_tick_t        init_tick;                         /**< timer timeout tick */
    rt_tick_t        timeout_tick;                      /**< timeout tick */
};
typedef struct rt_timer *rt_timer_t;

/*@}*/

/**
 * @addtogroup Signal
 */
#ifdef RT_USING_SIGNALS
#include <libc/libc_signal.h>
typedef unsigned long rt_sigset_t;
typedef void (*rt_sighandler_t)(int signo);
typedef siginfo_t rt_siginfo_t;

#define RT_SIG_MAX          32
#endif
/*@}*/

/**
 * @addtogroup Thread
 */

/*@{*/

/*
 * Thread
 */

/*
 * thread state definitions
 */
#define RT_THREAD_INIT                  0x00
#define RT_THREAD_READY                 0x01
#define RT_THREAD_SUSPEND               0x02
#define RT_THREAD_RUNNING               0x03
#define RT_THREAD_BLOCK                 RT_THREAD_SUSPEND
#define RT_THREAD_CLOSE                 0x04
#define RT_THREAD_STAT_MASK             0x0f

#define RT_THREAD_STAT_SIGNAL           0x10
#define RT_THREAD_STAT_SIGNAL_READY     (RT_THREAD_STAT_SIGNAL | RT_THREAD_READY)
#define RT_THREAD_STAT_SIGNAL_WAIT      0x20
#define RT_THREAD_STAT_SIGNAL_MASK      0xf0

/**
 * thread control command definitions
 */
#define RT_THREAD_CTRL_STARTUP          0x00
#define RT_THREAD_CTRL_CLOSE            0x01
#define RT_THREAD_CTRL_CHANGE_PRIORITY  0x02
#define RT_THREAD_CTRL_INFO             0x03

/**
 * Thread structure
 */
struct rt_thread
{
    /* rt object */
    char        name[RT_NAME_MAX];
    rt_uint8_t  type;
    rt_uint8_t  flags;

#ifdef RT_USING_MODULE
    void       *module_id;
#endif

    rt_list_t   list;                                   /**< the object list */
    rt_list_t   tlist;                                  /**< the thread list */

    /* stack point and entry */
    void       *sp;                                     /**< stack point */
    void       *entry;
    void       *parameter;
    void       *stack_addr;
    rt_uint32_t stack_size;

    /* error code */
    rt_err_t    error;

    rt_uint8_t  stat;                                   /**< thread status */

    /* priority */
    rt_uint8_t  current_priority;
    rt_uint8_t  init_priority;
#if RT_THREAD_PRIORITY_MAX > 32
    rt_uint8_t  number;
    rt_uint8_t  high_mask;
#endif
    rt_uint32_t number_mask;

#if defined(RT_USING_EVENT)
    /* thread event */
    rt_uint32_t event_set;
    rt_uint8_t  event_info;
#endif

#if defined(RT_USING_SIGNALS)
    rt_sigset_t     sig_pending;                        /**< the pending signals */
    rt_sigset_t     sig_mask;                           /**< the mask bits of signal */

    void            *sig_ret;                           /**< the return stack pointer from signal */
    rt_sighandler_t *sig_vectors;                       /**< vectors of signal handler */
    void            *si_list;                           /**< the signal infor list */
#endif

    rt_ubase_t  init_tick;                              /**< thread's initialized tick */
    rt_ubase_t  remaining_tick;                         /**< remaining tick */

    struct rt_timer thread_timer;                       /**< built-in thread timer */

    void (*cleanup)(struct rt_thread *tid);             /**< cleanup function when thread exit */

    /* light weight process if present */
#ifdef RT_USING_LWP
    void         *lwp;
#endif

    rt_uint32_t user_data;                             /**< private user data beyond this thread */
};
typedef struct rt_thread *rt_thread_t;

/*@}*/

/**
 * @addtogroup IPC
 */

/*@{*/

/**
 * IPC flags and control command definitions
 */
struct rt_ipc_object
{
    struct rt_object parent;                            /**< inherit from rt_object */

    rt_list_t        suspend_thread;                    /**< threads pended on this resource */
};

#ifdef RT_USING_SEMAPHORE
/**
 * Semaphore structure
 */
struct rt_semaphore
{
    struct rt_ipc_object parent;

    rt_uint16_t          value;
};
typedef struct rt_semaphore *rt_semaphore_t;
#endif

#ifdef RT_USING_MUTEX
/**
 * Mutual exclusion (mutex) structure
 */
struct rt_mutex
{
    struct rt_ipc_object parent;

    rt_uint16_t          value;

    rt_uint8_t           original_priority;             /**< priority of last thread hold the mutex */
    rt_uint8_t           hold;                          /**< numbers of thread hold the mutex */

    struct rt_thread    *owner;                         /**< current owner of mutex */
};
typedef struct rt_mutex * rt_mutex_t;
#endif

#ifdef RT_USING_EVENT
/**
 * flag defintions in event
 */
#define RT_EVENT_FLAG_AND               0x01
#define RT_EVENT_FALG_OR                0x02
#define RT_EVENT_FLAG_CLEAR             0x04

/*
 * event structure
 */
struct rt_event
{
    struct rt_ipc_object parent;

    rt_uint32_t          set;                           /**< event set */
};
typedef struct rt_event *rt_event_t;
#endif

#ifdef RT_USING_MAILBOX
/**
 * mailbox structure
 */
struct rt_mailbox
{
    struct rt_ipc_object parent;

    rt_uint32_t         *msg_pool;                      /**< start address of message buffer */

    rt_uint16_t          size;                          /**< size of message pool */

    rt_uint16_t          entry;                         /**< index of messages in msg_pool */
    rt_uint16_t          in_offset;                     /**< input offset of the message buffer */
    rt_uint16_t          out_offset;                    /**< output offset of the message buffer */

    rt_list_t            suspend_sender_thread;         /**< sender thread suspended on this mailbox */
};
typedef struct rt_mailbox *rt_mailbox_t;
#endif

#ifdef RT_USING_MESSAGEQUEUE
/**
 * message queue structure
 */
struct rt_messagequeue
{
    struct rt_ipc_object parent;                        /**< inherit from ipc_object */

    void                *msg_pool;                      /**< start address of message queue */

    rt_uint16_t          msg_size;                      /**< message size of each message */
    rt_uint16_t          max_msgs;                      /**< max number of messages */

    rt_uint16_t          entry;                         /**< index of messages in the queue */

    void                *msg_queue_head;                /**< list head */
    void                *msg_queue_tail;                /**< list tail */
    void                *msg_queue_free;                /**< pointer indicated the free node of queue */
};
typedef struct rt_messagequeue *rt_messagequeue;
#endif

/*@}*/

/**
 * @addtogroup MM
 */

/*@{*/

/*
 * memory management
 * heap & partition
 */
#ifdef RT_USING_MEMHEAP
/**
 * memory item on the heap
 */
struct rt_memheap_item
{
    rt_uint32_t             magic;                      /**< magic number for memheap */
    struct rt_memheap      *pool_ptr;                   /**< point of pool */

    struct rt_memheap_item *next;                       /**< next memheap item */
    struct rt_memheap_item *prev;                       /**< prev memheap item */

    struct rt_memheap_item *next_free;                  /**< next free memheap item */
    struct rt_memheap_item *prev_free;                  /**< prev free memheap item */
};

/**
 * Base structure of memory heap object
 */
struct rt_memheap
{
    struct rt_object        parent;

    void                   *strat_addr;                 /**< pool start address and size */

    rt_uint32_t             pool_size;
    rt_uint32_t             available_size;
    rt_uint32_t             max_used_size;              /**< maximum allocated size */

    struct rt_memheap_item *block_list;

    struct rt_memheap_item *free_list;
    struct rt_memheap_item *free_header;

    struct rt_semaphore     lock;
};
#endif

#ifdef RT_USING_MEMPOOL
/**
 * Base structure of Memory pool object
 */
struct rt_mempool
{
    struct rt_object parent;

    void            *start_address;                     /**< memory pool start */
    rt_size_t        size;                              /**< size of memory pool */

    rt_size_t        block_size;                        /**< size of memory blocks */
    rt_uint8_t      *block_list;                        /**< memory blocks list */

    rt_size_t        block_total_count;                 /**< numbers of memory block */
    rt_size_t        block_free_count;

    rt_list_t        suspend_thread;                    /**< threads pended on this resource */
    rt_size_t        suspend_thread_count;              /**< numbers of thread pended on this resource */

};
typedef struct rt_mempool *rt_mempool_t;
#endif

#ifdef RT_USING_DEVICE
/**
 * @addtogroup Device
 */

/*@{*/

/**
 * device (I/O) class type
 */
enum rt_device_class_type
{
    RT_Device_Class_Char = 0,
    RT_Device_Class_Block,
    RT_Device_Class_NetIf,
    RT_Device_Class_MTD,
    RT_Device_Class_CAN,
    RT_Device_Class_RTC,
    RT_Device_Class_Sound,
    RT_Device_Class_Graphic,
    RT_Device_Class_I2CBUS,
    RT_Device_Class_USBDevice,
    RT_Device_Class_USBHost,
    RT_Device_Class_SPIBUS,
    RT_Device_Class_SPIDevice,
    RT_Device_Class_SDIO,
    RT_Device_Class_PM,
    RT_Device_Class_Pipe,
    RT_Device_Class_Portal,
    RT_Device_Class_Timer,
    RT_Device_Class_Miscellaneous,
    RT_Device_Class_Unknown,
};

/**
 * device flags defitions
 */
#define RT_DEVICE_FLAG_DEACTIVATE       0x0000

#define RT_DEVICE_FLAG_RDONLY           0x0001
#define RT_DEVICE_FLAG_WRONLY           0x0002
#define RT_DEVICE_FLAG_RDWR             0x0003

#define RT_DEVICE_FLAG_REMOVABLE        0x0004
#define RT_DEVICE_FLAG_STANDALONE       0x0008
#define RT_DEVICE_FLAG_ACTIVATED        0x0010
#define RT_DEVICE_FLAG_SUSPENDED        0x0020
#define RT_DEVICE_FLAG_STREAM           0x0040

#define RT_DEVICE_FLAG_INT_RX           0x0100
#define RT_DEVICE_FLAG_DMA_RX           0x0200
#define RT_DEVICE_FLAG_INT_TX           0x0400
#define RT_DEVICE_FLAG_DMA_TX           0x0800

#define RT_DEVICE_OFLAG_CLOSE           0x0000
#define RT_DEVICE_OFLAG_RDONLY          0x0001
#define RT_DEVICE_OFLAG_WRONLY          0x0002
#define RT_DEVICE_OFLAG_RDWR            0x0003
#define RT_DEVICE_OFLAG_OPEN            0x0008
#define RT_DEVICE_OFLAG_MASK            0x0f0f

/**
 * general device commands
 */
#define RT_DEVICE_CTRL_RESUME           0x01
#define RT_DEVICE_CTRL_SUSPEND          0x02
#define RT_DEVICE_CTRL_CONFIG           0x03

#define RT_DEVICE_CTRL_SET_INT          0x10
#define RT_DEVICE_CTRL_CLR_INT          0x11
#define RT_DEVICE_CTRL_GET_INT          0x12

/**
 * special device commands
 */
#define RT_DEVICE_CTRL_CHAR_STREAM      0x10
#define RT_DEVICE_CTRL_BLK_GETGEOME     0x10
#define RT_DEVICE_CTRL_BLK_SYNC         0x11
#define RT_DEVICE_CTRL_BLK_ERASE        0x12
#define RT_DEVICE_CTRL_BLK_AUTOREFRESH  0x13
#define RT_DEVICE_CTRL_NETIF_GETMAC     0x10
#define RT_DEVICE_CTRL_MDT_FORMAT       0x10
#define RT_DEVICE_CTRL_RTC_GET_TIME     0x10
#define RT_DEVICE_CTRL_RTC_SET_TIME     0x11
#define RT_DEVICE_CTRL_RTC_GET_ALARM    0x12
#define RT_DEVICE_CTRL_RTC_SET_ALARM    0x13

typedef struct rt_device *rt_device_t;

/**
 * operations set for device object
 */
struct rt_device_ops
{
    /* common device interface */
    rt_err_t  (*init)    (rt_device_t dev);
    rt_err_t  (*open)    (rt_device_t dev, rt_uint16_t oflag);
    rt_err_t  (*close)   (rt_device_t dev);
    rt_size_t (*read)    (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);
    rt_size_t (*write)   (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);
    rt_err_t  (*control) (rt_device_t dev, int cmd, int *args);
};

/**
 * WaitQueue structure
 */
struct rt_wqueue
{
    rt_uint32_t flag;
    rt_list_t waiting_list;
};
typedef struct rt_wqueue *rt_wqueue_t;

/**
 * Device structure
 */
struct rt_device
{
    struct rt_object          parent;

    enum rt_device_class_type type;
    rt_uint16_t               flag;
    rt_uint16_t               open_flag;

    rt_uint8_t                ref_count;
    rt_uint8_t                device_id;                /**< 0 - 255 */

    /* device call back */
    rt_err_t (*rx_indicate) (rt_device_t dev, rt_size_t size);
    rt_err_t (*tx_complete) (rt_device_t dev, void *buffer);

#ifdef RT_USING_DEVICE_OPS
    const struct rt_device_ops *ops;
#else
    /* common device interface */
    rt_err_t  (*init)    (rt_device_t dev);
    rt_err_t  (*open)    (rt_device_t dev, rt_uint16_t oflag);
    rt_err_t  (*close)   (rt_device_t dev);
    rt_size_t (*read)    (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);
    rt_size_t (*write)   (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);
    rt_err_t  (*control) (rt_device_t dev, int cmd, int *args);
#endif

#if defined (RT_USING_POSIX)
    const struct dfs_file_ops *fops;
    struct rt_wqueue wait_queue;
#endif

    void                      *user_data;
};

/**
 * block device geometry structure
 */
struct rt_device_blk_geometry
{
    rt_uint32_t sector_count;
    rt_uint32_t bytes_per_sector;
    rt_uint32_t block_size;
};

/**
 * sector arrange struct on block device
 */
struct rt_device_blk_sectors
{
    rt_uint32_t sector_begin;
    rt_uint32_t sector_end;
};

/**
 * cursor control command
 */
#define RT_DEVICE_CTRL_CURSOR_SET_POSITION   0x10
#define RT_DEVICE_CTRL_CURSOR_SET_TYPE       0x11

/**
 * graphic device control command
 */
#define RTGRAPHIC_CTRL_RECT_UPDATE           0
#define RTGRAPHIC_CTRL_POWERON               1
#define RTGRAPHIC_CTRL_POWEROFF              2
#define RTGRAPHIC_CTRL_GET_INFO              3
#define RTGRAPHIC_CTRL_MODE                  4
#define RTGRAPHIC_CTRL_EXT                   5

/* graphic deice */
enum
{
    RTGRAPHIC_PIXEL_FORMAT_MONO = 0,
    RTGRAPHIC_PIXEL_FORMAT_GRAY4,
    RTGRAPHIC_PIXEL_FORMAT_GRAY16,
    RTGRAPHIC_PIXEL_FORMAT_RGB332,
    RTGRAPHIC_PIXEL_FORMAT_RGB444,
    RTGRAPHIC_PIXEL_FORMAT_RGB565,
    RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_BGR565 = RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_RGB666,
    RTGRAPHIC_PIXEL_FORMAT_RGB888,
    RTGRAPHIC_PIXEL_FORMAT_ARGB888,
    RTGRAPHIC_PIXEL_FORMAT_ABGR888,
    RTGRAPHIC_PIXEL_FORMAT_ARGB565,
    RTGRAPHIC_PIXEL_FORMAT_ALPHA,
};

/**
 * build a pixel position according to (x, y) coordinates.
 */
#define RTGRAPHIC_PIXEL_POSITION(x, y)    ((x << 16) | y)

/**
 * graphic device information structure
 */
struct rt_device_graphic_info
{
    rt_uint8_t  pixel_format;
    rt_uint8_t  bits_per_pixel;
    rt_uint16_t reserved;

    rt_uint16_t width;
    rt_uint16_t height;

    rt_uint8_t  *framebuffer;
};

/**
 * rectangle information structure
 */
struct rt_device_rect_info
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint16_t width;
    rt_uint16_t height;
};

/**
 * graphic operations
 */
struct rt_device_graphic_ops
{
    void (*set_pixel)  (const char *pixel, int x, int y);
    void (*get_pixel)  (char *pixel, int x, int y);
    
    void (*draw_hline) (const char *pixel, int x1, int x2, int y);
    void (*draw_vline) (const char *pixel, int x, int y1, int y2);

    void (*blit_line)  (const char *pixel, int x, int y, rt_size_t size);
};
#define rt_graphic_ops(device)     ((struct rt_device_graphic_ops *)(device->user_data))

/*@}*/
#endif /* end of RT_USING_DEVICE */

/* definitions for libc */
#include "rtlibc.h"

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/* RT-Thread definitions for C++ */
namespace rtthread {

enum TICK_WAIT {
    WAIT_NONE = 0,
    WAIT_FOREVER = -1,
};

}

#endif /* end of __cplusplus */

#endif
