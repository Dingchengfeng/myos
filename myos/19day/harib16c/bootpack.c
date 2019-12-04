

#include"bootpack.h" //""表示头文件在源文件目录下
#include<stdio.h> //<>表示头文件在编译器目录下
#include <string.h>

struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void console_task(struct SHEET *sheet, unsigned int memtotal);
int cons_newline(int cursor_y, struct SHEET *sheet);
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);

#define KEYCMD_LED		0xed //要向键盘发送的数据前缀

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化启动信息的结构体
	struct FIFO32 fifo, keycmd;
	struct SHTCTL *shtctl;
	char s[40];
	int fifobuf[128], keycmd_buf[32];
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;//内存信息的结构体
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;	
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	struct TASK *task_a, *task_cons;
	struct TIMER *timer;
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	init_gdtidt();//初始化段号表，中断向量表
	init_pic();
	io_sti();  //idt,pic初始化完成后，允许中断	
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); //设置PIT、PIC0、键盘中断许可（11111000）
	io_out8(PIC1_IMR, 0xef); //设置鼠标中断许可（11101111）
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */ //632K
	memman_free(memman, 0x00400000, memtotal - 0x00400000); //28M = 28672K

	init_palette(); //初始化调色板色表,就是建立编号到颜色的索引
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);//初始化图层控制器
	task_a = task_init(memman);//任务控制器初始化,并把当前程序构造成一个任务
	fifo.task = task_a;//当前任务休眠后，可被唤醒
	task_run(task_a, 1, 2);/* level=1, priority=2 */

	//sht_back
	sht_back  = sheet_alloc(shtctl); //分配背景图层
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny); //分配背景图层内存
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //不透明
	init_screen8(buf_back, binfo->scrnx, binfo->scrny); //初始化屏幕的内存
	
	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 不透明 */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level=2, priority=2 落后task_a执行*/
	
	/* sht_win*/
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); //不透明
	make_window8(buf_win, 144, 52, "task_a", 1); //初始化窗口
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	
	/* sht_mouse */	
	sht_mouse = sheet_alloc(shtctl); //分配鼠标图层
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); //透明度99
	init_mouse_cursor8(buf_mouse, 99);//初始化鼠标图形
	mx = (binfo->scrnx - 16) / 2; //16是鼠标本身的长
	my = (binfo->scrny - 28 - 16) / 2;//16是鼠标本身的宽，28是底部导航栏的宽
	//设置个图层位置
	sheet_slide(sht_back, 0, 0); //设置鼠标初始位置
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my);//设置鼠标位置
	sheet_updown(sht_back,  0); //背景图层在0层,显示刷新
	sheet_updown(sht_cons,  1); //命令行图层
	sheet_updown(sht_win,   2); //输入框
	sheet_updown(sht_mouse, 3); //鼠标图层
	
	/* 为了避免和键盘当前状态冲突，在一开始就进行设置 */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);
	
	for (;;) {//先处理键盘，再处理鼠标，再处理超时
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* 如果存在向键盘控制器发送的数据，则发送它 */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);//当前任务休眠，切换到其他任务后，就会中断使能，所以可以吧sti放后面，键盘中断会唤醒这个任务
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) {
				if (i < 0x80 + 256) {//将按键编码转换为字符编码
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {	/*当前输入为英文字母 */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
							((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;	/* 将大写转换为小写 */
					}
				}
				if (s[0] != 0) { /* 通常文字 */
					if (key_to == 0) {	/* 发送给任务a的 */
						if (cursor_x < 128) {
							/* 显示字符后光标后移 */
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	/* 发送给命令行的 */
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) { /* 退格键 */
					if (key_to == 0) {
						if (cursor_x > 8){
							/* 用空格把光标消去后，后移一次光标 */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else { /* 发送给命令行 */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}	
				}
				if (i == 256 + 0x1c) {	/* Enter */
					if (key_to != 0) {	/* 发送至命令行 */
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x0f) { /* Tab */
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1; /* 不显示光标 */
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);						
						fifo32_put(&task_cons->fifo, 2); //命令行串口光标ON
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000; /* 显示光标 */
						fifo32_put(&task_cons->fifo, 3); //命令行串口光标OFF
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x2a) {	/* 左shitf ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* 右shitf ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* 左shitf OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	/* 右shitf OFF */
					key_shift &= ~2;
				}
								if (i == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa) {	/* 键盘成功接收数据 */
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {	/* 键盘没有成功接收数据 */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				/* 光标再显示 */
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}	
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* 鼠标指针的移动 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sheet_slide(sht_mouse, mx, my); /* 移动鼠标位置 */
					if ((mdec.btn & 0x01) != 0) {
						/* 左键按下，移动sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			}else if (i <= 1) {//光标模拟，1s闪烁一次,0.5秒亮，0.5秒暗
				if(i != 0){
					timer_init(timer, &fifo, 0); /* 下次中断输出设置0 */
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				} else{
					timer_init(timer, &fifo, 1); /* 下次中断输出设置1 */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}	
			}													    
		}	
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	char s[30], cmdline[30], *p;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int x, y;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);//从磁盘镜像的0x002600位置开始记录文件信息
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	
	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));//读取fat（文件分配表）

	/* 显示提示符 */
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);
	
	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /* 光标定时器 */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /*置0 */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* 置1 */
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if (i == 2) {	/* 光标ON */
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* 光标OFF */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) { /* 键盘数据(通过任务A) */
				if (i == 8 + 256) {
					/* 退格键 */
					if (cursor_x > 16) {
						/* 用空白擦除光标后，将光标前移一位 */
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* 用空格将光标擦除 */
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;//命令行输入结尾
					cursor_y = cons_newline(cursor_y, sheet);
					/* コマンド実行 */
					if (strcmp(cmdline, "mem") == 0) {
						/* mem命令 */
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (strcmp(cmdline, "cls") == 0) {
						/* cls命令*/
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					} else if (strcmp(cmdline, "dir") == 0) {
						/* dir命令 */
						for (x = 0; x < 224; x++) {//最多存放224个文件信息
							if (finfo[x].name[0] == 0x00) {//文件名是0x00,表示不是文件信息，即从此之后没有文件
								break;
							}
							if (finfo[x].name[0] != 0xe5) {//0xe5表示删除的文件
								if ((finfo[x].type & 0x18) == 0) {//不是磁盘和目录
									sprintf(s, "filename.ext   %7d", finfo[x].size);//先记录大小，filename.ext是临时占位符，后面会覆盖成真实名称
									for (y = 0; y < 8; y++) {
										s[y] = finfo[x].name[y];
									}
									s[ 9] = finfo[x].ext[0];
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);
					}  else if (strncmp(cmdline, "type ", 5) == 0) {
						/* type命令 */
						/* 准备文件名 */
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						y = 0;
						for (x = 5; y < 11 && cmdline[x] != 0; x++) {
							if (cmdline[x] == '.' && y <= 8) {
								y = 8;
							} else {
								s[y] = cmdline[x];
								if ('a' <= s[y] && s[y] <= 'z') {
									/* 输入的小写字母转大写，和磁盘数据一致 */
									s[y] -= 0x20;
								} 
								y++;
							}
						}
						/* 寻找文件 */
						for (x = 0; x < 224; ) {
							if (finfo[x].name[0] == 0x00) {
								break;
							}
							if ((finfo[x].type & 0x18) == 0) {
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										goto type_next_file;
									}
								}
								break; /* 找到文件 */
							}
		type_next_file:
							x++;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							/* 找到文件的情况 */
							p = (char *) (finfo[x].clustno * 512 + 0x003e00 + ADR_DISKIMG);
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));							
							cursor_x = 8;
							for (y = 0; y < finfo[x].size; y++) {
								/* 逐字节输出 */
								s[0] = p[y];
								s[1] = 0;
								if (s[0] == 0x09) {	/* 制表符 */
									for (;;) {
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;	/* 被32整除就break */
										}
									}
								} else if (s[0] == 0x0a) {	/* 换行 */
									cursor_x = 8;//回车
									cursor_y = cons_newline(cursor_y, sheet);//换行
								} else if (s[0] == 0x0d) {	/* 回车 */
									/* 无操作 */
								} else {	/* 一般文字 */
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									if (cursor_x == 8 + 240) {
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							memman_free_4k(memman, (int) p, finfo[x].size);
						}  else {
							/* 没找到文件的情况 */
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					} else if (cmdline[0] != 0) {
						/* 错误输入 */
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					/* 显示提示符 */
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;//x坐标回车
				} else {
					/* 一般字符 */
					if (cursor_x < 240) {
						/* 显示一个字符之后，将光标后移一位 */
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			/* 重新显示光标 */
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

int cons_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) {
		cursor_y += 16; /* 换行 */
	} else {
		/* 滚动 */
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}
//fat：分配给fat的内存指针，img：fat的地址
void file_readfat(int *fat, unsigned char *img)
/* 将磁盘镜像中的fat解压缩 */
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}
