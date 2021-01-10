/**
 * Copyright (c) [2019-2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vprintf.h"
#include "vpool.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

int uprintf(const char *format, ...)
{
    int     ret;
    va_list valist;

    va_start(valist, format);
    ret = vuprintf(format, valist);
    va_end(valist);

    return ret;
}

v_weak int vuprintf(const char *format, va_list args)
{
    return vprintf(format, args);
}

v_weak int vputc(int ch)
{
    return putchar(ch);
}

v_weak int vgetc(void)
{
    return getchar();
}

/* defines */

#define XPF_ITYPE_INT       0
#define XPF_ITYPE_LONG      1
#define XPF_ITYPE_LONGLONG  2

/* typedefs */

struct xprintf_flags {
#define XPF_ALIGN_LEFT      0
#define XPF_ALIGN_RIGHT     1
    uint8_t align;
    uint8_t width;
    uint8_t pad;
    uint8_t base;
    char    sign;
    union {
        const char *       str;
        unsigned long long ull;
        char               ch;
    };
};

v_inline void __xprint_str(const char *str,
                           int (*output)(uintptr_t, unsigned char),
                           uintptr_t outarg)
{
    char ch;

    while ((ch = *str++) != '\0') {
        output(outarg, ch);

        if (ch == '\n') {
            output(outarg, '\r');
        }
    }
}

/**
 * @brief output a string
 *
 * @param xpf     [IN] the xprintf flags
 * @param output  [IN] output handler
 * @param outarg  [IN] argument to handler
 *
 * @return The number of characters output
 */
static int xprintf_str(struct xprintf_flags *xpf,
                       int (*output)(uintptr_t, unsigned char),
                       uintptr_t outarg)
{
    int len = (int)strlen(xpf->str);

    if (xpf->align == XPF_ALIGN_LEFT) {
        __xprint_str(xpf->str, output, outarg);

        while (len < xpf->width) {
            output(outarg, ' ');
            len++;
        }
    } else {
        if (len < xpf->width) {
            int i;

            for (i = 0; i < xpf->width - (uint8_t)len; i++) {
                output(outarg, xpf->pad);
            }
        }

        __xprint_str(xpf->str, output, outarg);
    }

    return max(len, xpf->width);
}

/**
 * @brief output a unsigned int number
 *
 * @param xpf     [IN] the xprintf flags
 * @param charset [IN] handler for args as they're formatted
 * @param output  [IN] output handler
 * @param outarg  [IN] argument to handler
 *
 * @return The number of characters output
 */
static int xprintf_nr(struct xprintf_flags *xpf, const char *charset,
                      int (*output)(uintptr_t, unsigned char),
                      uintptr_t outarg)
{
    char               buff[24];
    char             * p = &buff[sizeof(buff) - 1];
    int                len;
    unsigned long long ull = xpf->ull;

    *p = '\0';

    if (ull == 0) {
        *--p = '0';
        len  = 1;
    } else {
        len = 0;

        while (ull) {
            *--p = charset[ull % xpf->base];
            ull /= xpf->base;
            len++;
        }
    }

    if (xpf->sign != '\0') {
        *--p = xpf->sign;
        len++;
    }

    xpf->str = p;

    return xprintf_str(xpf, output, outarg);
}

static int xprintf_char(struct xprintf_flags *xpf,
                        int (*output)(uintptr_t, unsigned char),
                        uintptr_t outarg)
{
    char buff[4] = {xpf->ch, '\0'};

    xpf->str = buff;

    return xprintf_str(xpf, output, outarg);
}

#define GET_INTEGER_VAL(type)                               \
    do {                                                    \
        union {                                             \
            type     sv;                                    \
            unsigned type uv;                               \
        } u;                                                \
        u.uv    = va_arg(args, unsigned type);              \
        xpf.ull = u.uv;                                     \
                                                            \
        if ((!issigned) || (xpf.base != 10)) {              \
            break;                                          \
        }                                                   \
                                                            \
        if (unlikely(u.sv < 0)) {                           \
            xpf.sign = '-';                                 \
            xpf.ull  = (unsigned long long)-u.sv;           \
        }                                                   \
    } while (0)

/**
 * @brief convert a format string
 *
 * @param format  [IN] format string
 * @param args    [IN] pointer to varargs list
 * @param output  [IN] output handler
 * @param outarg  [IN] argument to handler
 *
 * @return The number of characters output
 */
int xprintf(const char *format, va_list args,
            int (*output)(uintptr_t, unsigned char),
            uintptr_t outarg)
{
    unsigned char        ch;
    char                 itype;
    int                  outputs = 0;
    char               * charset;
    bool                 issigned;
    struct xprintf_flags xpf;
    union {
        unsigned int ui;
        int          si;
    } ui;

    while ((likely(ch = *format++)) != '\0') {
        if (likely(ch != '%')) {
            outputs += output(outarg, ch);

            if (ch == '\n') {
                output(outarg, '\r');
            }

            continue;
        }

        xpf.pad   = ' ';
        xpf.sign  = '\0';
        xpf.align = XPF_ALIGN_RIGHT;
        xpf.width = 1;
        xpf.base  = 10;

        itype    = XPF_ITYPE_INT;
        charset  = NULL;
        issigned = true;

    reswitch:

        switch (ch = *format++) {
        case '\0':
            return outputs;
        case '%':
            outputs += output(outarg, ch);
            break;
        case 'l':
            if (*format == 'l') {
                format++;
                itype = XPF_ITYPE_LONGLONG;
            } else {
                itype = XPF_ITYPE_LONG;
            }

            goto reswitch;
        case 'p':
            outputs += output(outarg, '0');
            outputs += output(outarg, 'x');
            if (xpf.width == 1) {
                xpf.width = sizeof(uintptr_t) * 2;
            } else if (xpf.width > 2) {
                xpf.width -= 2;
            }

            xpf.pad = xpf.align == XPF_ALIGN_LEFT ? ' ' : '0';
        case 'x':
            charset = "0123456789abcdef";
        case 'X':
            xpf.base = 16;
            xpf.sign = '\0';
        case 'u':
            issigned = false;
        case 'd':
            charset = charset == NULL ? "0123456789ABCDEF" : charset;

            switch (itype) {
            case XPF_ITYPE_INT:
                GET_INTEGER_VAL(int);
                break;
            case XPF_ITYPE_LONG:
                GET_INTEGER_VAL(long);
                break;
            case XPF_ITYPE_LONGLONG:
                GET_INTEGER_VAL(long long);
                break;
            }

            outputs += xprintf_nr(&xpf, charset, output, outarg);
            break;
        case '*':
            ui.si = va_arg(args, int);
            if (ui.si >= 0) {
                xpf.width = (uint8_t)ui.si;
                goto reswitch;
            }
            xpf.width = (uint8_t)-ui.si;
        case '-':
            xpf.align = XPF_ALIGN_LEFT;
            goto reswitch;
        case '0':
            xpf.pad = ch;
        case ' ':
            goto reswitch;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            xpf.width = 0;
            do {
                xpf.width = xpf.width * 10 + ch - '0';
            } while (isdigit(ch = *format++));
            format--;
            goto reswitch;
        case 's':
            xpf.str = va_arg(args, const char *);
            outputs += xprintf_str(&xpf, output, outarg);
            break;
        case 'c':
            xpf.pad = ' ';
            xpf.ch  = (char)va_arg(args, int);
            outputs += xprintf_char(&xpf, output, outarg);
            break;
        case '+':
            xpf.sign = '+';
            goto reswitch;
        default:
            /* exit once meet a unsupported specifier, to aviod potential crash */
            return outputs;
        }
    }

    return outputs;
}

static void print_indent(FILE *stream, int num)
{
    int i;

    for (i = 0; i < num; ++i) {
        fprintf(stream, "    ");
    }
}

/**
 * @brief print buffer contents in message format
 *
 * uint8_t buffer[] = {
 *     1, 2, 3, 4, 43, 32, 1, 1, 144, 88, 4, 6, 7, 8, 9, 12,
 *     13, 14, 15, 16, 16, 235, 7, 8, 99, 54
 * };
 *
 * vprint_buffer(stdout, buffer, sizeof(buffer), 0);
 *
 * 01 02 03 04  2B 20 01 01  90 58 04 06  07 08 09 0C   ....+ ...X......
 * 0D 0E 0F 10  10 EB 07 08  63 36                      ........c6
 *
 * @param stream    [IN] the file stream, like: stdout, stderr
 * @param buffer    [IN] the data buffer
 * @param length    [IN] the length of buffer
 * @param indent    [IN] the indent count (each indent is four space)
 *
 * @return NA
 */
void vprint_buffer(FILE *stream, const uint8_t *buffer, int length, int indent)
{
    int i;

    if (length == 0) {
        fprintf(stream, "\n");
    }
    if (buffer == NULL) {
        return;
    }

    i = 0;
    while (i < length) {
        uint8_t array[16];
        int     j;

        print_indent(stream, indent);
        memcpy(array, buffer + i, 16);
        for (j = 0; j < 16 && i + j < length; j++) {
            fprintf(stream, "%02X ", array[j]);
            if (j % 4 == 3) {
                fprintf(stream, " ");
            }
        }
        if (length > 16) {
            while (j < 16) {
                fprintf(stream, "   ");
                if (j % 4 == 3) {
                    fprintf(stream, " ");
                }
                j++;
            }
        }
        fprintf(stream, " ");
        for (j = 0; j < 16 && i + j < length; j++) {
            if (isprint(array[j])) {
                fprintf(stream, "%c", array[j]);
            } else {
                fprintf(stream, ".");
            }
        }
        fprintf(stream, "\n");
        i += 16;
    }
}
