#include "bootpack.h"


void init_gdtidt(void)
{
	//从0x00270000-0x0027ffff的64K内存用来存放段号表（8192*8）
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	//从0x0026f800-0x0026f8ff的2K内存用来存放中断向量表（256*8）
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	/* GDT初始化 */
	for (i = 0; i < LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);//1号段，大小4G，段基址是0，表示CPU所能管理的内存本身
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, AR_CODE32_ER);//2号段，大小512K,段基址是0x00280000,是我们运行bootpack.hrb用的
	load_gdtr(LIMIT_GDT, ADR_GDT);//给GDTR赋值（段号表寄存器），6字节寄存器 0xffff00270000

	/* IDT初始化 */
	for (i = 0; i < LIMIT_IDT / 8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);//给IDTR赋值（中断向量号表寄存器），6字节寄存器0x07ff0026f800

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