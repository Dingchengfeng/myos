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

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	//通知PIC0 IRQ-01的受理已经完成，这样PIC0就会继续监视IRQ-01中断
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* PS/2鼠标中断处理程序 */
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);//通知PIC1 IRQ-12的受理已经完成
	io_out8(PIC0_OCW2, 0x62);//通知PIC0 IRQ-02的受理已经完成
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}
//对一部分机型而言，随着PIC的初始化，会产生一次IRQ中断，如果不对该中断处理程序执行STI,操作系统的启动会失败。
void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知 */
	return;
}
