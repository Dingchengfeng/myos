
void io_hlt(void);
void io_cli(void);
void io_out8(int port,int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);

//就算写在同一个源文件里，如果想要在定义前使用，还是必须先声明一下
void init_palette(void);
void set_palette(int palette_no, int color_num, unsigned char *rgb);

void HariMain(void)
{
	int i; 	
	char *p; //p所指内存存储8位元素
	//init_palette(); //设定调色板
	
	p = (char *) 0xa0000; //将地址赋值给p
	for(i = 0;i<=0xffff;i++){
		p[i] = i & 0x0f;
	}

	for(;;){
		io_hlt();
	}
	
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int palette_no, int color_num, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 记录中断许可标志 */
	io_cli(); 					/* 将中断许可标志设为0，禁止中断 */
	io_out8(0x03c8, palette_no);//调色板编号
	for (i = 0; i <= color_num; i++) {
		io_out8(0x03c9, rgb[0] / 4);//一种说法是VGA的RGB每种原色用6bit表示
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 复原中断许可标志 */
	return;
}
