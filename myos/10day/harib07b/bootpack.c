

#include<stdio.h> //<>表示头文件在编译器目录下
#include"bootpack.h" //""表示头文件在源文件目录下

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化结构体
	char s[40],keybuf[32],mousebuf[128];
	int mx, my,i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];	
	
	init_gdtidt();//初始化段号表
	init_pic();
	io_sti();  //idt,pic初始化完成后，允许中断	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); //PIC0中断许可
	io_out8(PIC1_IMR, 0xef); //PIC1中断许可
	
	init_keyboard();
	enable_mouse(&mdec);//激活鼠标
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */ //632K
	memman_free(memman, 0x00400000, memtotal - 0x00400000); //28M = 28672K

	init_palette(); //初始化调色板色表,就是建立编号到颜色的索引
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl); //分配背景图层
	sht_mouse = sheet_alloc(shtctl); //分配鼠标图层
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny); //分配背景图层内存
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //不透明
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); //透明度99
	init_screen8(buf_back, binfo->scrnx, binfo->scrny); //初始化屏幕
	init_mouse_cursor8(buf_mouse, 99);//初始化鼠标图形
	sheet_slide(shtctl, sht_back, 0, 0); //刷新鼠标初始位置
	mx = (binfo->scrnx - 16) / 2; //16是鼠标本身的长
	my = (binfo->scrny - 28 - 16) / 2;//16是鼠标本身的宽，28是底部导航栏的宽
	sheet_slide(shtctl, sht_mouse, mx, my);//刷新鼠标位置
	sheet_updown(shtctl, sht_back,  0); //背景图层在0层
	sheet_updown(shtctl, sht_mouse, 1); //鼠标图层在1层
	sprintf(s, "(%3d, %3d)", mx, my);//鼠标坐标
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); //背景中显示鼠标坐标
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);//内存信息
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s); //背景中显示内存信息
	sheet_refresh(shtctl); //刷新显示

	for (;;) {//先处理键盘，如果键盘为空在处理鼠标
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);//上次显示的位置复原背景色
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);//显示按键字符
				sheet_refresh(shtctl);//刷新显示
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* 三个字节凑齐就显示 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					/* 鼠标指针的移动 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 上次显示的位置复原背景色 */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示鼠标坐标 */
					sheet_slide(shtctl, sht_mouse, mx, my); /* 移动鼠标位置 */
				}
			}
		}
	}
}
