#include<stdio.h> //<>表示头文件在编译器目录下
#include"bootpack.h" //""表示头文件在源文件目录下



struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化结构体
	char s[40],mcursor[256],keybuf[32],mousebuf[128];
	int mx, my,i;
	struct MOUSE_DEC mdec;
	
	init_gdtidt();//初始化段号表
	init_pic();
	io_sti();  //idt,pic初始化完成后，允许中断	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
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
	
	enable_mouse(&mdec);
	
	for (;;) {//先处理键盘，如果键盘为空在处理鼠标
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
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
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
				}
			}
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

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//如果往键盘控制端口发送0xd4,下一条数据就会发送给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);//往键盘数据端口发送鼠标激活指令
	/* 顺利的话，键盘控制器会向cpu发送ACK(0xfa) */
	mdec->phase = 0;
	return; 
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* 等待鼠标初始化 */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 等待第一字节 */
		if ((dat & 0xc8) == 0x08) {
			/* 如果第一字节正确 */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* 等待第二字节 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* 等待第三字节 */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;//低三位对应鼠标按键
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {//这一位表示x方向有移动
			mdec->x |= 0xffffff00;//保留x方向的8位，其余置1
		}
		if ((mdec->buf[0] & 0x20) != 0) {//Yf方向同理
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* 鼠标的y方向与画面方向相反 */
		return 1;
	}
	return -1; /* 应该不可能到这里 */
}
