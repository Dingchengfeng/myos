/* ��???��L�꘢������?�I�������C��?���ȝQ�ꉺ*/

void io_hlt(void);
void write_mem8(int addr,int data);


void HariMain(void)
{
	int i;
	char *p; //*p���w�����n����? BYTE ?�x?����
	for(i = 0xa0000;i<=0xaffff;i++){
		p = i;
		*p = i & 0x0f;
	}

	for(;;){
		io_hlt();
	}	

}
