#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_MODULE
#include <dlmodule.h>
#endif

/* use precision */
#define RT_PRINTF_PRECISION

/**
 * @addtogroup KernelService
 */

/**@{*/

/* global errno in RT-Thread */
static volatile int __rt_errno;

#if defined(RT_USING_DEVIDE) && defined(RT_USING_CONSOLE)
static rt_device_t _console_device = RT_NULL;
#endif

/*
 * This function will get errno
 *
 * @return errno
 */
rt_err_t rt_get_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        return __rt_errno;
    }

    tid = rt_thread_self();
    if (tid == RT_NULL)
        return __rt_errno;

    return tid->error;
}
RTM_EXPORT(rt_get_errno);

/*
 * This function will set errno
 *
 * @param error the errno shall be set
 */
void rt_set_errno(rt_err_t error)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        __rt_errno = error;

        return;
    }

    tid = rt_thread_self();
    if(tid == RT_NULL)
    {
        __rt_errno = error;

        return;
    }

    tid->error = error;
}
RTM_EXPORT(rt_set_errno);

/**
 * This function returns errno.
 *
 * @return the errno in the system
 */
int *_rt_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
        return (int *)&__rt_errno;

    tid = rt_thread_self();
    if (tid != RT_NULL)
        return (int *) & (tid->error);

    return (int *)&__rt_errno;
}
RTM_EXPORT(_rt_errno);

/**
 * This function will set the content of memory to specified value
 *
 * @param s the address of source memory
 * @param c the value shall be set in content
 * @param count the copied length
 *
 * @return the address of source memory
 */
void *rt_memset(void *s, int c, rt_ubase_t count)
{
#ifdef RT_USING_TINY_SIZE
    char *xs = (char *)s;

    while(count--)
        *xs++ = c;

    return s;
#else
#define LBLOCKSIZE      (sizeof(rt_int32_t))
#define UNALIGNED(X)    ((rt_int32_t)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN)  ((LEN) < LBLOCKSIZE)

    int i;
    char *m = (char *)s;
    rt_uint32_t buffer;
    rt_uint32_t *aligned_addr;
    rt_uint32_t d = c & 0xff;

    if (!TOO_SAMLL(count) && !UNALIGNED(s))
    {
        /* If we get this far, we know that n is large and m is word-aligned. */
        aligned_addr = (rt_uint32_t *)s;

        /* Store D into each char sized location in BUFFER so that
         * we can set large blocks quickly.
         */
        if (LBLOCKSIZE == 4)
        {
            buffer = (d << 8) | d;
            buffer |= (buffer << 16);
        }
        else
        {
            buffer = 0;
            for (i = 0; i < LBLOCKSIZE; i++)
                buffer = (buffer << 8) | d;
        }

        while (count >= (LBLOCKSIZE << 2))
        {
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            count -= (LBLOCKSIZE << 2);
        }

        while (count >= LBLOCKSIZE)
        {
            *aligned_addr++ = buffer;
            count -= LBLOCKSIZE;
        }

        /* Pick up the remainder with a bytewise loop. */
        m = (char *)aligned_addr;
    }

    while (count--)
    {
        *m++ = (char)d;
    }

    return s;

#undef LBLOCKSIZE
#undef UNALIGNED
#undef TOO_SAMLL
#endif
}
RTM_EXPORT(rt_memset);

/**
 * This function will copy memory content from source address to destination
 * address.
 *
 * @param dst the address of destination memory
 * @param src  the address of source memory
 * @param count the copied length
 *
 * @return the address of destination memory
 */
void *rt_memcpy(void *dst, const void *src, rt_ubase_t count)
{
#ifdef RT_USING_TINY_SIZE
    char *tmp = (char *)dst, *s = (char *)src;
    rt_ubase_t len;

    if(tmp < s || tmp > (s + count))
    {
        while (count--)
            *tmp ++ = *s ++;
    }
    else
    {
        for (len = count; len > 0; len --)
            tmp[len - 1] = s[len - 1];
    }

    return dst;
#else

#define UNALIGNED(X, Y) \
                        (((rt_int32_t)X & (sizeof(rt_int32_t) - 1)) | \
                         ((rt_int32_t)Y & (sizeof(rt_int32_t) - 1)))
#define BIGBLOCKSIZE    (sizeof(rt_int32_t) << 2)
#define LITTLEBLOCKSIZE (sizeof(rt_int32_t))
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    rt_int32_t *aligned_dst;
    rt_int32_t *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(dst_ptr, src_ptr))
    {
        aligned_dst = (rt_int32_t *)dst_ptr;
        aligned_src = (rt_int32_t *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE)
        {
            *aligned_dst ++ = *aligned_src ++;
            *aligned_dst ++ = *aligned_src ++;
            *aligned_dst ++ = *aligned_src ++;
            *aligned_dst ++ = *aligned_src ++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE)
        {
            *aligned_dst ++ = *aligned_src ++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (char *)aligned_src;
    }

    while (len--)
        *dst_ptr ++ = *src_ptr ++;

    return dst;
#undef BIGBLOCKSIZE
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
#endif
}
RTM_EXPORT(rt_memcpy);

/**
 * This function will move memory content from source address to destination
 * address.
 *
 * @param dest the address of destination memory
 * @param src  the address of source memory
 * @param n the copied length
 *
 * @return the address of destination memory
 */
void *rt_memmove(void *dst, const void *src, rt_ubase_t n)
{
    char *tmp = (char *)dst, *s = (char *)src;

    if (tmp <= s || tmp > s + n)
    {
        while (n--)
            *(tmp++) = *(s++);
    }
    else
    {
        tmp += n;
        s += n;

        while (n--)
            *(--tmp) = *(--s);
    }

    return dst;
}
RTM_EXPORT(rt_memmove);

/**
 * This function will compare two areas of memory
 *
 * @param cs one area of memory
 * @param ct znother area of memory
 * @param count the size of the area
 *
 * @return the result
 */
rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_ubase_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = cs, su2 = ct; count >0; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;

    return res;
}
RTM_EXPORT(rt_memcmp);

/**
 * This function will return the first occurrence of a string.
 *
 * @param s1 the source string
 * @param s2 the find string
 *
 * @return the first occurrence of a s2 in s1, or RT_NULL if no found.
 */
char *rt_strstr(const char *s1, const char *s2)
{
    int len_s1, len_s2;

    len_s2 = rt_strlen(s2);
    if (!len_s2)
        return (char *)s1;

    len_s1 = rt_strlen(s1);
    while (len_s1 >= len_s2)
    {
        len_s1--;
        if (!rt_memcmp(s1, s2, len_s2))
            return (char *)s1;
        s1++;
    }

    return RT_NULL;
}
RTM_EXPORT(rt_strstr);

/**
 * This function will compare two strings while ignoring differences in case
 *
 * @param a the string to be compared
 * @param b the string to be compared
 *
 * @return the result
 */
rt_uint32_t rt_strcasecmp(const char *a, const char *b)
{
    int ca, cb;

    do
    {
        ca = *a++ & 0xff;
        cb = *b++ & 0xff;

        if (ca >= 'A' && ca <= 'Z')
            ca += 'a' - 'A';
        if (cb > 'A' && cb <= 'Z')
            cb += 'a' - 'A';
    } while (ca == cb && ca != '\0');

    return (ca - cb);
}
RTM_EXPORT(RT_strcasecmp);

/**
 * This function will copy string no more than n bytes.
 *
 * @param dst the string to copy
 * @param src the string to be copied
 * @param n the maximum copied length
 *
 * @return the result
 */
char *rt_strncpy(char *dst, const char *src, rt_ubase_t n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;

        do
        {
            if ((*d++ = *s++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                    *d++ = 0;
                break;
            }
        } while (--n != 0);
    }

    return dst;
}
RTM_EXPORT(rt_strncpy);

/**
 * This function will compare two strings with specified maximum length
 *
 * @param cs the string to be compared
 * @param ct the string to be compared
 * @param count the maximum compare length
 *
 * @return the result
 */
rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_ubase_t count)
{
    register unsigned char __res = 0;

    while (count--)
    {
        if ((__res = *cs - *cs++) != 0 || !cs++)
            break;
    }

    return __res;
}
RTM_EXPORT(rt_strncmp);

/**
 * This function will compare two strings without specified length
 *
 * @param cs the string to be compared
 * @param ct the string to be compared
 *
 * @return the result
 */
rt_int32_t rt_strcmp(const char *cs, const char *ct)
{
    while (*cs && *cs == *ct)
        cs++, ct++;

    return (*cs - *ct);
}
RTM_EXPORT(rt_strcmp);

/**
 * The  strnlen()  function  returns the number of characters in the
 * string pointed to by s, excluding the terminating null byte ('\0'),
 * but at most maxlen.  In doing this, strnlen() looks only at the
 * first maxlen characters in the string pointed to by s and never
 * beyond s+maxlen.
 *
 * @param s the string
 * @param maxlen the max size
 * @return the length of string
 */
rt_size_t rt_strnlen(const char *s, rt_ubase_t maxlen)
{
    const char *sc;

    for (sc = s; *sc != '\0' && sc - s < maxlen; sc++)
        ; /* nothing */

    return (sc - s);
}

/**
 * This function will return the length of a string, which terminate will
 * null character.
 *
 * @param s the string
 *
 * @return the length of string
 */
rt_size_t rt_strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; sc++)
        ; /* nothing */

    return (sc - s);
}
RTM_EXPORT(rt_strlen);

#ifdef RT_USING_HEAP
/**
 * This function will duplicate a string.
 *
 * @param s the string to be duplicated
 *
 * @return the duplicated string pointer
 */
char *rt_strdup(const char *s)
{
    rt_size_t len = rt_strlen(s) + 1;
    char *tmp = (char *)rt_malloc(len);

    if (!tmp)
        return RT_NULL;

    rt_memcpy(tmp, s, len);

    return tmp;
}
RTM_EXPORT(rt_strdup);
#ifdef __CC_ARM
char *strdup(const char *s) __attribute__((alias("rt_strdup")));
#endif
#endif

/**
 * This function will show the version of rt-thread rtos
 */
void rt_show_version(void)
{
    rt_kprintf("\n \\ | /\n");
    rt_kprintf("- RT -     Thread Operating System\n");
    rt_kprintf(" / | \\     %d.%d.%d build %s\n",
               RT_VERSION, RT_SUBVERSION, RT_REVISION, __DATE__);
    rt_kprintf(" 2006 - 2018 Copyright by rt-thread team\n");
    rt_kprintf(" This version is typed by hotwater99.\n");
}
RTM_EXPORT(rt_show_version);

rt_inline rt_int32_t divide(rt_int32_t *n, rt_int32_t base)
{
    rt_int32_t res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10)
    {
        res = ((rt_uint32_t) * n) % 10U;
        *n = ((rt_uint32_t) * n) / 10U；
    }
    else
    {
        res = ((rt_uint32_t) * n) % 16U；
        *n = ((rt_uint32_t) * n) / 16U;
    }

    return res;
}

rt_inline int skip_atoi(const char **s)
{
    register int i = 0;
    while (isdigit(**s))
        i = i * 10 + *(*s++) - '0';

    return i;
}

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */

#ifdef RT_PRINTF_PRECISION
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   precision,
                          int   type)
#else
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   type)
#endif
{
    char c, sign;
#ifdef RT_PRINTF_LONG
    char tmp[32];
#else
    char tmp[16];
#endif
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int i;
    register int size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

    ////////////////////////////

}

rt_int32_t rt_vsnprintf(char       *buf,
                        rt_size_t   size,
                        const char *fmt,
                        va_list     args)
{

}
RTM_EXPORT(rt_vsnprintf);

/**
 * This function will fill a formatted string to buffer
 *
 * @param buf the buffer to save formatted string
 * @param size the size of buffer
 * @param fmt the format
 */
rt_int32_t rt_snprintf(char *buf, rt_size_t size, const char *fmt, ...)
{
    rt_int32_t n;
    va_list args;

    va_start(args, fmt);
    n = rt_vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

/**
 * This function will fill a formatted string to buffer
 *
 * @param buf the buffer to save formatted string
 * @param arg_ptr the arg_ptr
 * @param format the format
 */
rt_int32_t rt_vsprintf(char *buf, const char *format, va_list arg_ptr)
{
    return rt_vsnprintf(buf, (rt_size_t) - 1, format, arg_ptr);
}
RTM_EXPORT(rt_vsprintf);

/**
 * This function will fill a formatted string to buffer
 *
 * @param buf the buffer to save formatted string
 * @param format the format
 */
rt_int32_t rt_sprintf(char *buf, const char *format, ...)
{
    rt_int32_t n;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    n = rt_vsprintf(buf, format, arg_ptr);
    va_end(arg_ptr);

    return n;
}
RTM_EXPORT(rt_sprintf);

#ifdef RT_USING_CONSOLE

#ifdef RT_USING_DEVICE
/**
 * This function returns the device using in console.
 *
 * @return the device using in console or RT_NULL
 */
rt_device_t rt_console_get_device(void)
{
    return _console_device;
}
RTM_EXPORT(rt_console_get_device);

/**
 * This function will set a device as console device.
 * After set a device to console, all output of rt_kprintf will be
 * redirected to this new device.
 *
 * @param name the name of new console device
 *
 * @return the old console device handler
 */
rt_device_t rt_console_set_device(const char *name)
{
    rt_device_t new, old;

    /* save old device */
    old = _console_device;

    /* find new console device */
    new = rt_device_find(name);
    if (new != RT_NULL)
    {
        if (_console_device != RT_NULL)
        {
            /* close old console device */
            rt_device_close(_console_device);
        }

        /* set new console device */
        rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        _console_device = new;
    }

    return old;
}
RTM_EXPORT(rt_console_set_device);
#endif

RT_WEAK void rt_hw_console_output(const char *str)
{
    /* empty console output */
}
RTM_EXPORT(rt_hw_console_output);

/**
 * This function will put string to the console.
 *
 * @param str the string output to the console.
 */
void rt_kputs(const char *str)
{
    if (!str) return;

#ifdef RT_USING_DEVICE
    if (_console_device != RT_NULL)
    {
        rt_hw_console_output(str);
    }
    else
    {
        rt_uint16_t old_flag = _console_device->open_flag;

        _console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
        rt_device_write(_console_device, 0, str, rt_strlen(str));
        _console_device->open_flag = old_flag;
    }
#else
    rt_hw_console_output(str);
#endif
}

/**
 * This function will print a formatted string on system console
 *
 * @param fmt the format
 */
void rt_kprintf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length,
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];

    va_strat(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
#ifdef RT_USING_DEVICE
    if (_console_device == RT_NULL)
    {
        rt_hw_console_output(rt_log_buff);
    }
    else
    {
        rt_uint16_t old_flag = _console_device->open_flag;

        _console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
        rt_device_write(_console_device, 0, rt_log_buf, length);
        _console_device->open_flag = old_flag;
    }
#else
    rt_hw_console_output(rt_log_buf);
#endif
    va_end(args);
}
RTM_EXPORT(rt_kprintf);
#endif

#ifdef RT_USING_HEAP
/**
 * This function allocates a memory block, which address is aligned to the
 * specified alignment size.
 *
 * @param size the allocated memory block size
 * @param align the alignment size
 *
 * @return the allocated memory block on successful, otherwise returns RT_NULL
 */





#endif










