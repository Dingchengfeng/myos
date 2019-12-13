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
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_asm_inthandler20, _asm_inthandler21
		GLOBAL  _asm_inthandler27, _asm_inthandler2c
		GLOBAL	_asm_inthandler0d
		GLOBAL	_memtest_sub
		GLOBAL	_farjmp, _farcall
		GLOBAL	_asm_hrb_api, _start_app
		EXTERN	_inthandler20, _inthandler21
		EXTERN	_inthandler27, _inthandler2c
		EXTERN	_inthandler0d		
		EXTERN	_hrb_api		

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

_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET
		
_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET
		
_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		; ���������`��
		JNE		end_app		; ���������`��
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d �Ǥϡ����줬��Ҫ
		IRETD
		
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; �����ڻ�Ҫʹ��EBX, ESI, EDI��
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start; 12��ָǰ���3����ջ��4�Ƿ������ص�ַ
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR	[ESP+4]				; JMP FAR ���ȶ�ȡָ���ڴ��4���ֽ�����(eip)������EIP,�ټ�����ȡ2�ֽ����ݣ�����CS��
		RET	
		
_farcall:		; void farcall(int eip, int cs);
		CALL	FAR	[ESP+4]				; eip, cs
		RET
		
_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		; ���ڱ����PUSH
		PUSHAD		; ������hrb_api��ֵ��PUSH
		MOV		AX,SS
		MOV		DS,AX		; ��os�õĶε�ַ����DS��ES
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; EAX��Ϊ0ʱ����Ӧ�ó���
		JNE		end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
end_app:
;	EAXΪtss.esp0�ĵ�ַ
		MOV		ESP,[EAX]
		POPAD
		RET					; ����cmd_app
		
_start_app:		; void start_app(int eip, int cs, int esp, int ds);
		PUSHAD		; ��8��ͨ�üĴ�����ֵȫ����������,
		MOV		EAX,[ESP+36]	; Ӧ�ó����õ�EIP
		MOV		ECX,[ESP+40]	; Ӧ�ó����õ�CS
		MOV		EDX,[ESP+44]	; Ӧ�ó����õ�ESP
		MOV		EBX,[ESP+48]	; Ӧ�ó����õ�DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0�ĵ�ַ
		MOV		[EBP  ],ESP		; ����os��ESP
		MOV		[EBP+4],SS		;����os��SS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	�������ջ���Ա���RETF��ת��Ӧ�ó���
		OR		ECX,3			; ��Ӧ�ó����õĶκź�3����or����
		OR		EBX,3			; ��Ӧ�ó����õĶκź�3����or����
		PUSH	EBX				; Ӧ�ó����SS
		PUSH	EDX				; Ӧ�ó����ESP
		PUSH	ECX				; Ӧ�ó����CS
		PUSH	EAX				; Ӧ�ó����EIP
		RETF
;	Ӧ�ó�������󲻻�ص�����	
		