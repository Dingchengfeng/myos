/* 告???器有一个函数在?的文件里，麻?自己找一下*/

void io_hlt(void);
void write_mem8(int addr,int data);


void HariMain(void)
{
	int i;
	char *p; //*p所指内存地址存? BYTE ?度?数据
	for(i = 0xa0000;i<=0xaffff;i++){
		p = i;
		*p = i & 0x0f;
	}

	for(;;){
		io_hlt();
	}	

}
