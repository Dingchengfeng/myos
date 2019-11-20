

#include<stdio.h> //<>表示头文件在编译器目录下
#include"bootpack.h" //""表示头文件在源文件目录下

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

struct TSS32 {//任务状态段
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

void task_b_main(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;//初始化启动信息的结构体
	struct FIFO32 fifo;
	char s[40];
	int fifobuf[128];
	struct TIMER *timer, *timer2, *timer3;
	int mx, my, i, cursor_x, cursor_c, task_b_esp;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;//内存信息的结构体
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;	
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	struct TSS32 tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;	//初始化段号表信息
	
	init_gdtidt();//初始化段号表（和上面重复了），中断向量表
	init_pic();
	io_sti();  //idt,pic初始化完成后，允许中断	
	fifo32_init(&fifo, 128, fifobuf);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); //设置PIT、PIC0、键盘中断许可（11111000）
	io_out8(PIC1_IMR, 0xef); //设置鼠标中断许可（11101111）
	//初始化定时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */ //632K
	memman_free(memman, 0x00400000, memtotal - 0x00400000); //28M = 28672K

	init_palette(); //初始化调色板色表,就是建立编号到颜色的索引
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);//初始化图层控制器
	sht_back  = sheet_alloc(shtctl); //分配背景图层
	sht_mouse = sheet_alloc(shtctl); //分配鼠标图层
	sht_win   = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny); //分配背景图层内存
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //不透明
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); //透明度99
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); //不透明
	init_screen8(buf_back, binfo->scrnx, binfo->scrny); //初始化屏幕的内存
	init_mouse_cursor8(buf_mouse, 99);//初始化鼠标图形
	make_window8(buf_win, 160, 52, "window"); //初始化窗口
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	sheet_slide(sht_back, 0, 0); //设置鼠标初始位置
	mx = (binfo->scrnx - 16) / 2; //16是鼠标本身的长
	my = (binfo->scrny - 28 - 16) / 2;//16是鼠标本身的宽，28是底部导航栏的宽
	sheet_slide(sht_mouse, mx, my);//设置鼠标位置
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0); //背景图层在0层,显示刷新
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2); //鼠标图层在2层,显示刷新
	sprintf(s, "(%3d, %3d)", mx, my);//鼠标坐标
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10); //背景中显示鼠标坐标
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);//内存信息
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);
	load_tr(3 * 8); //记录当前当前任务Tss段号,任务切换时会自动段变化
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024; //栈底地址
	tss_b.eip = (int) &task_b_main; //指令指针
	tss_b.eflags = 0x00000202; /* IF = 1; */
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp; //栈指针寄存器，指向栈顶，现在栈顶和栈底一样
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8; //代码段寄存器,现在代码确实在这个段
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	
	for (;;) {//先处理键盘，再处理鼠标，再处理超时
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 256 + 0x54) {
					if (keytable[i - 256] != 0&& cursor_x < 144) {//通用字符
						//显示一个字符就迁移一次光标
						s[0] = keytable[i - 256];
						s[1] = 0;//s数组输出结尾标志是0
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				if (i == 256 + 0x0e && cursor_x > 8) { /* 退格键 */
					/* 用空格把光标消去后，后移一次光标 */
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				/* 光标再显示 */
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i - 512) != 0) {
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
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
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
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my); /* 移动鼠标位置 */
					if ((mdec.btn & 0x01) != 0) {
						/* 左键按下，移动sht_win */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if(i == 10){//10秒超时
					putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
					taskswitch4();
			}else if(i == 3){//3秒超时
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			}else if (i <= 1) {//光标模拟，1s闪烁一次,0.5秒亮，0.5秒暗
				if(i != 0){
					timer_init(timer3, &fifo, 0); /* 下次中断输出设置0 */
					cursor_c = COL8_000000;
				} else{
					timer_init(timer3, &fifo, 1); /* 下次中断输出设置1 */
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}													    
		}	
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
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
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
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

void task_b_main(void)
{
	for (;;) { io_hlt(); }
}
