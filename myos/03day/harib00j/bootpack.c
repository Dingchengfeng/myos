/*告诉编译器，有一个函数在别的文件里*/
void io_hlt(void);
void HariMain(void)
{

fin:
	/* 这里想用HLT,但C语言中不能用HLT!*/
	goto fin;

}
