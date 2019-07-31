
void io_hlt(void);

void HariMain(void)
{
	int i; 	
	char *p;
	p = (char *) 0xa0000; //将地址赋值给p
	for(i = 0;i<=0xffff;i++){
		p[i] = i & 0x0f;
	}

	for(;;){
		io_hlt();
	}	

}
