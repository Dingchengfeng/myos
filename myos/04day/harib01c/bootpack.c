
void io_hlt(void);


void HariMain(void)
{
	int i;
	char *p; /*表示指针所指定内存是 byte类型 ，能存储8bit内容。但是指针p本身是32位的*/
	for(i = 0xa0000;i<=0xaffff;i++){
		p = i;
		*p = i & 0x0f;
	}

	for(;;){
		io_hlt();
	}	

}
