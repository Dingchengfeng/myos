#include "bootpack.h"


void init_gdtidt(void)
{
	//��0x00270000-0x0027ffff��64K�ڴ�������Ŷκű�8192*8��
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	//��0x0026f800-0x0026f8ff��2K�ڴ���������ж�������256*8��
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	/* GDT��ʼ�� */
	for (i = 0; i < LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);//1�ŶΣ���С4G���λ�ַ��0����ʾCPU���ܹ�����ڴ汾��
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, AR_CODE32_ER);//2�ŶΣ���С512K,�λ�ַ��0x00280000,����������bootpack.hrb�õ�
	load_gdtr(LIMIT_GDT, ADR_GDT);//��GDTR��ֵ���κű�Ĵ�������6�ֽڼĴ��� 0xffff00270000

	/* IDT��ʼ�� */
	for (i = 0; i < LIMIT_IDT / 8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);//��IDTR��ֵ���ж������ű�Ĵ�������6�ֽڼĴ���0x07ff0026f800

	/* IDT�趨 */
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {//1M
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}