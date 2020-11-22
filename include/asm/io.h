// 硬件端口字节输出
// value: 欲输出字节
#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

// 硬件端口字节输入
// 返回读取的字节
#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})

// 以下两个不同处在于加了两条跳转语句延迟

// 带延迟的硬件端口字节输出
#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

// 带延迟的硬件端口输入函数
#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})
