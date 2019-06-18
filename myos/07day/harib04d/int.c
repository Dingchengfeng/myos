#include "bootpack.h"
#include <stdio.h>

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

#define PORT_KEYDAT		0x0060

struct KEYBUF keybuf;

void inthandler21(int *esp)
/* PS/2�����жϴ������*/
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x61); //IRQ-01������ɵ�֪ͨ������PIC�ͻ��������IRQ1�ж�
	data = io_in8(PORT_KEYDAT);
	if (keybuf.len<32) {
		keybuf.data[keybuf.next_w] = data;
		keybuf.len++;
		keybuf.next_w++;
		if(keybuf.next_w == 32){
			keybuf.next_w = 0;
		}
	}
	return;

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
//��һ���ֻ��Ͷ��ԣ�����PIC�ĳ�ʼ���������һ��IRQ�жϣ�������Ը��жϴ������ִ��STI,����ϵͳ��������ʧ�ܡ�
void inthandler27(int *esp)
/* PIC0����β���ȫ����z�ߌ��� */
/* Athlon64X2�C�ʤɤǤϥ��åץ��åȤζ��Ϥˤ��PIC�γ��ڻ��r�ˤ��θ���z�ߤ�1�Ȥ��������� */
/* ���θ���z�߄I���v���ϡ����θ���z�ߤˌ����ƺΤ⤷�ʤ��Ǥ���^���� */
/* �ʤ��Τ⤷�ʤ��Ƥ����Σ�
	��  ���θ���z�ߤ�PIC���ڻ��r��늚ݵĤʥΥ����ˤ�äưk��������ΤʤΤǡ�
		�ޤ���˺Τ��I���Ƥ���Ҫ���ʤ���									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07�ܸ����ˤ�PIC��֪ͨ(7-1����) */
	return;
}
