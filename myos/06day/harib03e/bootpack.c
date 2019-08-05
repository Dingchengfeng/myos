#include<stdio.h> //<>表示头文件在编译器目录下
#include"bootpack.h" //""表示头文件在源文件目录下

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化结构体
	char s[40],mcursor[256];
	int mx, my;
	
	init_gdtidt();//初始化段号表，中断向量表
	init_pic();//初始化PIC
	io_sti(); //cpu中断使能
	
	init_palette(); //初始化调色板色表,就是建立编号到颜色的索引
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny); //初始化屏幕
	mx = (binfo->scrnx - 16) / 2; //16是鼠标本身的长
	my = (binfo->scrny - 28 - 16) / 2;//16是鼠标本身的宽，28是底部导航栏的宽
	init_mouse_cursor8(mcursor, COL8_008484);//初始化鼠标图形
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);//显示鼠标图形
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	io_out8(PIC0_IMR, 0xf9); //1111 1001 接收键盘和PIC1中断
	io_out8(PIC1_IMR, 0xef); //1110 1111 接收鼠标中断
	
	for (;;) {
		io_hlt();
	}
}





