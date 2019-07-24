

#include "bootpack.h"

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* PS/2鼠标中断处理程序 */
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);//通知PIC1 IRQ-12的受理已经完成
	io_out8(PIC0_OCW2, 0x62);//通知PIC0 IRQ-02的受理已经完成
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
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