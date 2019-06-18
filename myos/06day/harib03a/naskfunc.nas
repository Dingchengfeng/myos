; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 	
[INSTRSET "i486p"]				; 
[BITS 32]						; 
[FILE "naskfunc.nas"]			;

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr

[SECTION .text]					;[SECTION .text]	��ʾ����ο�ʼ [SECTION .data]	��ʾ���ݶο�ʼ

_io_hlt:	; void io_hlt(void);
		HLT		;ʹ����ֹͣ���У�������������ͣ״̬����ִ���κβ�������Ӱ���־����RESET�����и�λ�źš�CPU��Ӧ�������жϡ�CPU��Ӧ�������ж�3�����֮һʱ��CPU������ͣ״̬��ִ��HLT����һ��ָ��
		RET

_io_cli:	; void io_cli(void);
		CLI		;��ֹ�жϷ����������޸�flag�Ĵ������жϱ�־λ��
		RET

_io_sti:	; void io_sti(void);
		STI		;�����ж�	�������޸�flag�Ĵ������жϱ�־λ��
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX					;��DX����ֵ��Ӧ�Ķ˿ڶ���AL�ܴ����λ��������8λ��
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX					;��DX����ֵ��Ӧ�Ķ˿ڶ���AX�ܴ����λ��������16λ��
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL					;��AL������������DX����ֵ��Ӧ�Ķ˿�	
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS ��EFLAGS�Ĵ�����32λ��������ѹ��ջ
		POP		EAX	;��ջ�е������ݴ���EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX	;��EAX����ѹ��ջ
		POPFD		; POP EFLAGS ��ջ�е���4�ֽڴ���EFLAGS
		RET
;��Դ�������е�ֵ���ص�ȫ�����������Ĵ��� (GDTR) ���ж����������Ĵ��� (IDTR)��Դ������ָ�� 6 �ֽ��ڴ�λ�ã�������ȫ����������� (GDT) ���ж���������� (IDT) �Ļ�ַ�����Ե�ַ�������ƣ�����С�����ֽڼƣ��������������С������ 32 λ���� 16 λ���ƣ�6 �ֽ����ݲ������� 2 ����λ�ֽڣ��� 32 λ��ַ�����ݲ������� 4 ����λ�ֽڣ����ص��Ĵ����������������С������ 16 λ������� 16 λ���ƣ�2 ����λ�ֽڣ��� 24 λ��ַ���������ġ����ֽڣ��������ʹ�ò������ĸ�λ�ֽڣ�GDTR �� IDTR �л�ַ�ĸ�λ�ֽ�������䡣		
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET


