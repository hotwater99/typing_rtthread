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
