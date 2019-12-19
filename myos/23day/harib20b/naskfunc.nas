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
		GLOBAL	_asm_inthandler0c, _asm_inthandler0d
		GLOBAL	_asm_end_app, _memtest_sub
		GLOBAL	_farjmp, _farcall
		GLOBAL	_asm_hrb_api, _start_app
		EXTERN	_inthandler20, _inthandler21
		EXTERN	_inthandler27, _inthandler2c
		EXTERN	_inthandler0c, _inthandler0d	
		EXTERN	_hrb_api		

[SECTION .text]					;[SECTION .text]	表示程序段开始 [SECTION .data]	表示数据段开始

_io_hlt:	; void io_hlt(void);
		HLT		;使程序停止运行，处理器进入暂停状态，不执行任何操作，不影响标志。当RESET线上有复位信号、CPU响应非屏蔽中断、CPU响应可屏蔽中断3种情况之一时，CPU脱离暂停状态，执行HLT的下一条指令
		RET

_io_cli:	; void io_cli(void);
		CLI		;禁止中断发生（就是修改flag寄存器中中断标志位）
		RET

_io_sti:	; void io_sti(void);
		STI		;允许中断	（就是修改flag寄存器中中断标志位）
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX					;从DX里数值对应的端口读入AL能存入的位数的数（8位）
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX					;从DX里数值对应的端口读入AX能存入的位数的数（16位）
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL					;将AL里的数据输出到DX里数值对应的端口	
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
		PUSHFD		; PUSH EFLAGS 把EFLAGS寄存器（32位）的内容压入栈
		POP		EAX	;从栈中弹出数据存入EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX	;把EAX内容压入栈
		POPFD		; POP EFLAGS 从栈中弹出4字节存入EFLAGS
		RET
;将源操作数中的值加载到全局描述符表格寄存器 (GDTR) 或中断描述符表格寄存器 (IDTR)。源操作数指定 6 字节内存位置，它包含全局描述符表格 (GDT) 或中断描述符表格 (IDT) 的基址（线性地址）与限制（表格大小，以字节计）。如果操作数大小属性是 32 位，则将 16 位限制（6 字节数据操作数的 2 个低位字节）与 32 位基址（数据操作数的 4 个高位字节）加载到寄存器。如果操作数大小属性是 16 位，则加载 16 位限制（2 个低位字节）与 24 位基址（第三、四、五字节）。这里，不使用操作数的高位字节，GDTR 或 IDTR 中基址的高位字节用零填充。		
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

_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0
		JNE		_asm_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			
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
		CMP		EAX,0		
		JNE		_asm_end_app		
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d では、これが必要
		IRETD
		
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （由于还要使用EBX, ESI, EDI）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start; 12是指前面的3次入栈，4是方法返回地址
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
		JMP		FAR	[ESP+4]				; JMP FAR 会先读取指定内存的4个字节数据(eip)，存入EIP,再继续读取2字节数据，存入CS。
		RET	
		
_farcall:		; void farcall(int eip, int cs);
		CALL	FAR	[ESP+4]				; eip, cs
		RET
		
_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		; 用于保存的PUSH
		PUSHAD		; 用于向hrb_api传值得PUSH
		MOV		AX,SS
		MOV		DS,AX		; 将os用的段地址存入DS、ES
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; EAX不为0时结束应用程序
		JNE		_asm_end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD
_asm_end_app:
;	EAX为tss.esp0的地址
		MOV		ESP,[EAX]
		MOV		DWORD [EAX+4],0
		POPAD
		RET					; 返回cmd_app
		
_start_app:		; void start_app(int eip, int cs, int esp, int ds);
		PUSHAD		; 将8个通用寄存器的值全部保存起来,
		MOV		EAX,[ESP+36]	; 应用程序用的EIP
		MOV		ECX,[ESP+40]	; 应用程序用的CS
		MOV		EDX,[ESP+44]	; 应用程序用的ESP
		MOV		EBX,[ESP+48]	; 应用程序用的DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0的地址
		MOV		[EBP  ],ESP		; 保存os的ESP
		MOV		[EBP+4],SS		;保存os的SS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	下面调整栈，以便用RETF跳转到应用程序
		OR		ECX,3			; 将应用程序用的段号和3进行or运算
		OR		EBX,3			; 将应用程序用的段号和3进行or运算
		PUSH	EBX				; 应用程序的SS
		PUSH	EDX				; 应用程序的ESP
		PUSH	ECX				; 应用程序的CS
		PUSH	EAX				; 应用程序的EIP
		RETF
;	应用程序结束后不会回到这里	
		