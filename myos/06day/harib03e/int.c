#include "bootpack.h"

void init_pic(void)
/* PIC��ʼ�� */
{
	io_out8(PIC0_IMR,  0xff  ); /* ��ֹ�����ж� */
	io_out8(PIC1_IMR,  0xff  ); /* ��ֹ�����ж� */

	io_out8(PIC0_ICW1, 0x11  ); /* ���ش���ģʽ */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7��INT20-27���� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1��IRQ2���� */
	io_out8(PIC0_ICW4, 0x01  ); /* �޻���ģʽ */

	io_out8(PIC1_ICW1, 0x11  ); /* ���ش���ģʽ */            
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15��INT28-2f���� */ 
	io_out8(PIC1_ICW3, 2     ); /* PIC1��IRQ2���� */       
	io_out8(PIC1_ICW4, 0x01  ); /* �޻���ģʽ */             

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 IRQ2��PIC1������ȫ����ֹ */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ��ֹ�����ж� */

	return;
}

void inthandler21(int *esp)
/* PS/2�����жϴ������*/
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
	for (;;) {
		io_hlt();
	}
}

void inthandler2c(int *esp)
/* PS/2����жϴ������ */
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for (;;) {
		io_hlt();
	}
}
//��һ���ֻ��Ͷ��ԣ�����PIC�ĳ�ʼ���������һ��IRQ�жϣ�������Ը��жϴ������ִ��STI(�����жϱ�־λ),����ϵͳ��������ʧ�ܡ�
void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67);//֪ͨPIC IRQ-7�Ѿ�����
	return;
}
