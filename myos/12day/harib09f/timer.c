

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* 已配置状态 */
#define TIMER_FLAGS_USING		2	/* 定时器运行中 */
//10ms产生一次中断信号
void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* 最初的超时时刻 */
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timer[i].flags = 0; /* 未使用 */
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timer[i].flags == 0) {
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0; /* 没有可用的定时器 */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* 未使用 */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
		if (timerctl.next > timer->timeout) {
		/* 更新超时时刻 */
		timerctl.next = timer->timeout;
	}
	return;
}

void inthandler20(int *esp)
{
	int i;	
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	timerctl.count++;
		if (timerctl.next > timerctl.count) {
		return; /* 还没到下一个超时时刻，结束 */
	}
	timerctl.next = 0xffffffff;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
			//超时
			if (timerctl.timer[i].timeout <= timerctl.count) {
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			} else {
				/* 没有超时，找到下一个超时时刻 */
				if (timerctl.next > timerctl.timer[i].timeout) {
					timerctl.next = timerctl.timer[i].timeout;
				}
			}
		}
	}
	return;
}