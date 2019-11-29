
#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf,struct TASK *task)
/* FIFO初始化 */
{
	fifo->size = size;//缓冲区大小
	fifo->buf = buf;
	fifo->free = size; /* 缓冲区空闲大小 */
	fifo->flags = 0;
	fifo->p = 0; /* 下一个数据写入位置 */
	fifo->q = 0; /* 下一个数据读取位置 */
	fifo->task = task;//需要唤醒的任务
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* FIFO写数据 */
{
	if (fifo->free == 0) {
		/* 没有空闲空间，溢出 */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task != 0) {//接收中断后唤醒任务
		if (fifo->task->flags != 2) { /*如果任务不是运行态 */
			task_run(fifo->task); /* 将任务加入到待运行数组 */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* FIFO读数据 */
{
	int data;
	if (fifo->free == fifo->size) {
		/* 缓冲区为空，读取失败*/
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/* fifo里积攒的数据数 */
{
	return fifo->size - fifo->free;
}
