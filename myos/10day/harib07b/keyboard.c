

#include "bootpack.h"

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	//通知PIC0 IRQ-01的受理已经完成，这样PIC0就会继续监视IRQ-01中断
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

#define PORT_KEYSTA				0x0064 //键盘控制端口
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