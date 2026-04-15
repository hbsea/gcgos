#include "../include/types.h"
#include <stdarg.h>
#include "ulib.h"

static char digits[] = "0123456789abcdef";

static void putc(int fd, char c) { write(fd, &c, 1); }

static void printint(int fd, long long xx, int base, int sign)
{
    char buf[20];
    int i;
    unsigned long long x;
    if (sign && (sign = (xx < 0)))
        x = -xx;
    else
        x = xx;
    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while (
        (x /= base) !=
        0);  // 打印10会显示-0000000000000000000000000，原因是写成：while((x/base)!=0)
    if (sign) buf[i++] = '-';
    while (--i >= 0) putc(fd, buf[i]);
}
static void printptr(int fd, uint64 x)
{
    int i;
    putc(fd, '0');
    putc(fd, 'x');
    // 向右取每次最高4位,64位，总的16次
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// va_list是c内建的，保存“当前正在遍历到哪个可变参数”的状态
// va_start(ap, last_named_param)初始化
// ap，让它指向紧跟在最后一个具名形参后的第一个可变参数。必须在使用 ap
// 之前调用，且第二个参数必须是函数里最后一个具名形参的标识符。 读va_arg(ap,
// type)取下一个可变参数，并把 ap
// 向后推进,你必须告诉它要读取的类型（这点非常重要，错了就是未定义行为）。 Print
// to the console.
void printf(int fd, char* fmt, ...)
{
    va_list ap;
    int i, cx, c0, c1, c2;
    char* s;

    va_start(ap, fmt);
    for (i = 0; (cx = fmt[i] & 0xff) != 0; i++)
    {
        if (cx != '%')
        {
            putc(fd, cx);
            continue;
        }
        i++;
        c0 = fmt[i + 0] & 0xff;
        c1 = c2 = 0;
        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;
        if (c0 == 'd')
        {
            printint(fd, va_arg(ap, int), 10, 1);
        }
        else if (c0 == 'b')
        {
            printint(fd, va_arg(ap, uint64), 2, 1);
        }
        else if (c0 == 'l' && c1 == 'd')
        {
            printint(fd, va_arg(ap, uint64), 10, 1);
            i += 1;
        }
        else if (c0 == 'l' && c1 == 'l' && c2 == 'd')
        {
            printint(fd, va_arg(ap, uint64), 10, 1);
            i += 2;
        }
        else if (c0 == 'u')
        {
            printint(fd, va_arg(ap, uint32), 10, 0);
        }
        else if (c0 == 'l' && c1 == 'u')
        {
            printint(fd, va_arg(ap, uint64), 10, 0);
            i += 1;
        }
        else if (c0 == 'l' && c1 == 'l' && c2 == 'u')
        {
            printint(fd, va_arg(ap, uint64), 10, 0);
            i += 2;
        }
        else if (c0 == 'x' || c0 == 'p')
        {
            printint(fd, va_arg(ap, uint32), 16, 0);
        }
        else if (c0 == 'l' && c1 == 'x')
        {
            printint(fd, va_arg(ap, uint64), 16, 0);
            i += 1;
        }
        else if (c0 == 'l' && c1 == 'l' && c2 == 'x')
        {
            printint(fd, va_arg(ap, uint64), 16, 0);
            i += 2;
        }
        else if (c0 == 'p')
        {
            printptr(fd, va_arg(ap, uint64));
        }
        else if (c0 == 'c')
        {
            putc(fd, va_arg(ap, uint));
        }
        else if (c0 == 's')
        {
            if ((s = va_arg(ap, char*)) == 0) s = "(null)";
            for (; *s; s++) putc(fd, *s);
        }
        else if (c0 == '%')
        {
            putc(fd, '%');
        }
        else if (c0 == 0)
        {
            break;
        }
        else
        {
            // Print unknown % sequence to draw attention.
            putc(fd, '%');
            putc(fd, c0);
        }
    }
    va_end(ap);
}
