
#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf,struct TASK *task)
/* FIFO��ʼ�� */
{
	fifo->size = size;//��������С
	fifo->buf = buf;
	fifo->free = size; /* ���������д�С */
	fifo->flags = 0;
	fifo->p = 0; /* ��һ������д��λ�� */
	fifo->q = 0; /* ��һ�����ݶ�ȡλ�� */
	fifo->task = task;//��Ҫ���ѵ�����
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* FIFOд���� */
{
	if (fifo->free == 0) {
		/* û�п��пռ䣬��� */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task != 0) {//�����жϺ�������
		if (fifo->task->flags != 2) { /*�������������̬ */
			task_run(fifo->task, 0); /* ��������뵽���������� */
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* FIFO������ */
{
	int data;
	if (fifo->free == fifo->size) {
		/* ������Ϊ�գ���ȡʧ��*/
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
/* fifo����ܵ������� */
{
	return fifo->size - fifo->free;
}
