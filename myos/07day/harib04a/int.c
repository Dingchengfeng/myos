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

void inthandler21(int *esp)
/* PS/2键盘中断处理程序*/
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	unsigned char data,s[4];
	io_out8(PIC0_OCW2,0x61); //IRQ-01受理完成的通知，这样PIC就会继续监视IRQ1中断
	data = io_in8(PORT_KEYDAT);
	sprintf(s, "%02X", data);//两位16进制数，不足补0，
	boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);//用背景色覆盖这个位置，其实是覆盖上个字符。
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
	
	return;
}

void inthandler2c(int *esp)
/* PS/2鼠标中断处理程序 */
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for (;;) {
		io_hlt();
	}
}
//对一部分机型而言，随着PIC的初始化，会产生一次IRQ中断，如果不对该中断处理程序执行STI,操作系统的启动会失败。
void inthandler27(int *esp)
/* PIC0からの不完全割り込み対策 */
/* Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる */
/* この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす */
/* なぜ何もしなくていいの？
	→  この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
		まじめに何か処理してやる必要がない。									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
	return;
}
