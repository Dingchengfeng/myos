;hello-os
;TAB=4

		ORG		0x7c00			;bios��Ĭ�ϰ���������װ�ص������ַ��������αָ����߻�����������ڴ��ַ�����λ��ִ�У���������ʱ�������ַΪ��������ָ���£�$����Ҫ������ڴ��ַ��������ʾ��������һ���ֽ�������Ҳ���Ǽ���0x7c00						

;��׼FAT12��ʽ����ר�ô���

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		;�������������֣������������ַ�������8�ֽڣ�
		DW		512				;ÿ��������С������Ϊ512�ֽڣ�
		DB		1				;�صĴ�С������Ϊ1��������
		DW		1				;FAT����ʼλ�ã�һ��ӵ�1��������ʼ��
		DB		2				;FAT�ĸ���������Ϊ2��
		DW		224				;��Ŀ¼�Ĵ�С��һ�����ó�224�
		DW		2880			;�ô��̵Ĵ�С��������2880������
		DB		0xf0			;���̵����ࣨ������0xf0��
		DW		9				;FAT�ĳ��ȣ�������9������
		DW		18				;1���ŵ��м���������������18��
		DW		2				;��ͷ����������2��
		DD		0				;�Ƿ�ʹ�÷�����0��ʾ�񣬱�����0��
		DD		2880			;��дһ�δ��̴�С
		DB		0,0,0x29		;�������ֽ����岻�����������
		DD		0xffffffff		;�����Ǿ�����
		DB		"HELLO-OS   "	;�������ƣ�11�ֽ������ַ���
		DB		"FAT12   "		;���̸�ʽ���ƣ�8�ֽڣ�
		RESB	18				;�ճ�18�ֽڣ���18���ֽڻ���0��䣩					
	
;��������

entry:
		MOV		AX,0			; ��ʼ���Ĵ���
		MOV		SS,AX			;ջ�μĴ���
		MOV		SP,0x7c00		;ջָ��Ĵ���
		MOV		DS,AX			;���ݶμĴ���

;����

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; ����0
		MOV		DH,0			; ��ͷ0
		MOV		CL,2			; ����2
readloop:
		MOV		SI,0			; ��¼ʧ�ܴ����ļĴ���
retry:
		MOV		AH,0x02			; AH=0x02 : �������
		MOV		AL,1			; 1������
		MOV		BX,0			; ES*16+BX ��������ַ
		MOV		DL,0x00			; A������
		INT		0x13			; BIOS
		JNC		next			; û������ת
		ADD		SI,1			; ��SI��1
		CMP		SI,5			; �Ƚ�SI��5
		JAE		error			; SI >= 5 ����error
		MOV		AH,0x00			; AH=0x00 : 0x13����������	
		MOV		DL,0x00			; A������
		INT		0x13			; ����������
		JMP		retry
next:
		MOV		AX,ES
		ADD		AX,0x0020
		MOV		ES,AX			;ͨ��AX��ES����0x0020,���ǰ����ݻ����ַ������512�ֽ�
		ADD		CL,1			;��CL���1
		CMP		CL,18			;�Ƚ�CL��18
		JBE		readloop

;cpu˯��
	
fin:
		HLT						; ��CPUֹͣ���ȴ�ָ��
		JMP		fin				; ����ѭ��
		
error:
		MOV		SI,msg		
putloop:
		MOV		AL,[SI]			;Ҫ��ʾ������
		ADD		SI,1			; SI���ֵ����
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; INT 0x10��ʾ��������
		MOV		BX,15			; ָ���ַ���ɫ��0-255��
		INT		0x10			; �����Կ�BIOS
		JMP		putloop
msg:
		DB		0x0a, 0x0a		; 2������
		DB		"hello, world"
		DB		0x0a			; ����
		DB		0
		
		RESB	0x7dfe-$		; ��0��0x7dfe

		DB		0x55, 0xaa		;����������β��־
		