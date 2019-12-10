void api_end(void);

void HariMain(void)
{
	*((char *) 0x00102600) = 0;//直接改变系统数据段内容，这里应该是dir指令用的数据吧
	api_end();
}
