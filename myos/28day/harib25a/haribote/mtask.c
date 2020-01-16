

#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
//取出当前活动的task
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running++;//这里最好判断下是否超过最大数量
	task->flags = 2; /* 活动中 */
	return;
}

void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* 寻找task所在位置 */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			/* 找到了 */
			break;
		}
	}

	tl->running--;
	if (i < tl->now) {
		tl->now--; /* 移动成员 */
	}
	if (tl->now >= tl->running) {
		/* now修正 */
		tl->now = 0;
	}
	task->flags = 1; /* 休眠中 */

	/* 移动 */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}
//设置最优先的level
void task_switchsub(void)
{
	int i;
	/* 寻找最上层的level */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; /* 找到了 */
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}
//初始化任务控制器，并把当前程序做成一个任务
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));//分配任务控制器内存
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;//任务未分配标志
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
		set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int) taskctl->tasks0[i].ldt, AR_LDT);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}
	
	task = task_alloc();
	task->flags = 2; /* 运行中 */
	task->priority = 2; /* 默认0.02秒 */
	task->level = 0;//最高level，
	task_add(task);
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);//优先级是通过超时时间设置的
	
	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);
	
	return task;
}
//分配一个任务
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 正在使用标志 */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* 先设置为0 */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.iomap = 0x40000000;
			task->tss.ss0 = 0;
			return task;
		}
	}
	return 0; /* 全部使用中 */
}
//level越小，优先级越高，priority 越大，运行时间越长
void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* 不改变level */
	}	
	if (priority > 0) {
		task->priority = priority;
	}
	
	if (task->flags == 2 && task->level != level) { /* 有level改变 */
		task_remove(task); /* 先移除，这样下面的if就会执行 */
	}
	if (task->flags != 2) {
		//从休眠状态唤醒
		task->level = level;
		task_add(task);
	}
	
	taskctl->lv_change = 1;//下次任务切换时检查level
	return;
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2) {
		/* 处于活动状态 */
		now_task = task_now();
		task_remove(task); /* 执行此语句的话flags将变为1 */
		if (task == now_task) {
			/* 如果让自己休眠，则要进行任务切换 */
			task_switchsub();
			now_task = task_now(); /* 设定后，获取当前任务 */
			farjmp(0, now_task->sel);//切换任务
		}
	}
	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];	
	tl->now++;
	if (tl->now == tl->running) {//将要运行的任务是最后一个，now归0
		tl->now = 0;
	}
		if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);//优先级越高，当前任务运行时间越多
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}
