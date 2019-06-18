void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

//cpu�ڹ���GDT��8�ֽ�����
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
//cpu�ڹ���IDT��8�ֽ�����
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};


void init_gdtidt(void)
{
	//��0x00270000-0x0027ffff��64K�ڴ�������Ŷκű�8192*8��
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	//��0x0026f800-0x0026f8ff��2K�ڴ���������ж�������256*8��
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;
	int i;

	/* GDT��ʼ�� */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);//1�ŶΣ���С4G���λ�ַ��0����ʾCPU���ܹ�����ڴ汾��
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);//2�ŶΣ���С512K,�λ�ַ��0x00280000,����������bootpack.hrb�õ�
	load_gdtr(0xffff, 0x00270000);//��GDTR��ֵ���κű�Ĵ�������6�ֽڼĴ���

	/* IDT��ʼ�� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);//��IDTR��ֵ���ж������ű�Ĵ�������6�ֽڼĴ���

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