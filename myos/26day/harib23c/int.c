

#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PIC初始化 */
{
	io_out8(PIC0_IMR,  0xff  ); /* 禁止所有中断 */
	io_out8(PIC1_IMR,  0xff  ); /* 禁止所有中断 */

	io_out8(PIC0_ICW1, 0x11  ); /* 边沿触发模式 */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7由INT20-27接收 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1由IRQ2连接 */
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* 边沿触发模式 */            
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15由INT28-2f接收 */ 
	io_out8(PIC1_ICW3, 2     ); /* PIC1由IRQ2连接 */       
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲模式 */             

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 IRQ2（PIC1）以外全部禁止 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 禁止所有中断 */

	return;
}


//对一部分机型而言，随着PIC的初始化，会产生一次IRQ中断，如果不对该中断处理程序执行STI,操作系统的启动会失败。
void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知 */
	return;
}
