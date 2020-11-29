// 移动到用户模式运行
// 利用 iret 指令实现从内核模式移动到初始任务 0 中执行
// LDT 在 include/linux/sched.h INIT_TASK 处已经定义
#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \		// 保存 esp
	"pushl $0x17\n\t" \					// ss 入栈, 00010111b 段选择符, [1:0] 表示特权级 3, [2] 表示 LDT[2]
	"pushl %%eax\n\t" \					// esp
	"pushfl\n\t" \						// eflags
	"pushl $0x0f\n\t" \					// cs 入栈, 00001111b 段选择符, [1:0] 表示特权级 3, [2] 表示 LDT[1]
	"pushl $1f\n\t" \					// eip 入栈，1f 即表示下面标号 1 的偏移地址
	"iret\n" \							// 执行 iret 后就会跳转都标号 1 处
	"1:\tmovl $0x17,%%eax\n\t" \		// 此处开始执行任务 0
	"movw %%ax,%%ds\n\t" \				// 将 ds,es,fs,gs 都设置为跟 ss 一样
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)

// 设置门描述符 
// 根据中断或异常处理程序偏移地址 addr、门描述符类型 type 和特权级信息 dpl
// 设置位于 gate_addr 处的门描述符
// %0 由 dpl, type 组成的标识字
// %1 描述符低 4 bytes 地址
// %2 描述符高 4 bytes 地址
// edx 偏移地址 addr
// eax 高字含有段选择符 0x8
// 最后要使 eax : 0008, low(addr); edx : high(addr), 1<<15+dpl<<13+type<<8

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

// 按照段描述符的格式拼接数据
// base[31:24] G D/B 0 AVL limit[19:16]
// P DPL/2 S type/4 base[23:16]
// base[15:0]
// limit[15:0]
#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")

