#include<stdio.h> //<>表示头文件在编译器目录下
#include"bootpack.h" //""表示头文件在源文件目录下



extern struct FIFO8 keyfifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化结构体
	char s[40],mcursor[256],keybuf[32];
	int mx, my,i;
	
	init_gdtidt();//初始化段号表
	init_pic();
	io_sti();  //idt,pic初始化完成后，允许中断	
	fifo8_init(&keyfifo, 32, keybuf);
	io_out8(PIC0_IMR, 0xf9); //PIC0中断许可
	io_out8(PIC1_IMR, 0xef); //PIC1中断许可
	
	init_keyboard();
	
	init_palette(); //初始化调色板色表,就是建立编号到颜色的索引
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny); //初始化屏幕
	mx = (binfo->scrnx - 16) / 2; //16是鼠标本身的长
	my = (binfo->scrny - 28 - 16) / 2;//16是鼠标本身的宽，28是底部导航栏的宽
	init_mouse_cursor8(mcursor, COL8_008484);//初始化鼠标图形
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);//显示鼠标图形
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	enable_mouse();
	
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) == 0) {
			io_stihlt();
		} else {
			i = fifo8_get(&keyfifo);
			io_sti();
			sprintf(s, "%02X", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
}

#define PORT_KEYDAT				0x0060 //键盘数据端口
#define PORT_KEYSTA				0x0064 //键盘控制端口
#define PORT_KEYCMD				0x0064 //键盘控制端口
#define KEYSTA_SEND_NOTREADY	0x02 //判断键盘是否准备好的掩码
#define KEYCMD_WRITE_MODE		0x60   //设定键盘模式的指令
#define KBC_MODE				0x47       //利用鼠标模式的模式号

void wait_KBC_sendready(void)
{
	/*等待键盘控制电路准备完毕 */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {//若键盘控制端口倒数第二位是0，则键盘准备好了
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); //键盘控制电路准备完毕后，告诉键盘将要设定键盘模式
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);//键盘控制电路准备完毕后，设定键盘模式为利用鼠标模式
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//如果往键盘控制端口发送0xd4,下一条数据就会发送给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);//往键盘数据端口发送鼠标激活指令
	return; /* 顺利的话，键盘控制器会向cpu发送ACK(0xfa) */
}



