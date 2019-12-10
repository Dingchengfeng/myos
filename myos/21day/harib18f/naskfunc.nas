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
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app	;判断是系统态还是用户态
;	系统态下的中断
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	用户态下的中断
		MOV		EAX,1*8
		MOV		DS,AX			; 设置系统DS
		MOV		ECX,[0xfe4]		; OSd的ESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 保存中断时应用程序的SS
		MOV		[ECX  ],ESP		; 保存中断时应用程序的ESP
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler20
		POP		ECX
		POP		EAX
		MOV		SS,AX			; 恢复应用程序SS
		MOV		ESP,ECX			; 恢复应用程序ESP
		POPAD
		POP		DS
		POP		ES
		IRETD
		
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler21
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler27
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		IRETD
.from_app:						;点开头的是本地标签，即使和其他函数中标签重复，也能区分
;	アプリが動いているときに割り込まれた
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		CALL	_inthandler2c
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d では、これが必要
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた
		CLI
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		STI
		CALL	_inthandler0d	; 异常处理程序
		CLI
		CMP		EAX,0			; 异常处理程序返回值不为0，则强制结束应用
		JNE		.kill
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d では、これが必要
		IRETD
.kill:
;	强制结束应用程序
		MOV		EAX,1*8			; OS用のDS/SS
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]		; start_appのときのESPに無理やり戻す
		STI			; 切り替え完了なので割り込み可能に戻す
		POPAD	; 保存しておいたレジスタを回復
		RET
		
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
		; 这里因为是中断请求调用的，所以先保持中断禁止
		PUSH	DS
		PUSH	ES
		PUSHAD		; 保存用户态寄存器数据
		MOV		EAX,1*8
		MOV		DS,AX			; 设定系统代码段
		MOV		ECX,[0xfe4]		; OS的ESP
		ADD		ECX,-40
		MOV		[ECX+32],ESP	; 保存应用程序的ESP
		MOV		[ECX+36],SS		; 保存应用程序的SS

; 将PUSHAD后的值复制到系统栈
		MOV		EDX,[ESP   ]
		MOV		EBX,[ESP+ 4]
		MOV		[ECX   ],EDX	; 复制传递给hrb_api
		MOV		[ECX+ 4],EBX	; 复制传递给hrb_api
		MOV		EDX,[ESP+ 8]
		MOV		EBX,[ESP+12]
		MOV		[ECX+ 8],EDX	; 复制传递给hrb_api
		MOV		[ECX+12],EBX	; 复制传递给hrb_api
		MOV		EDX,[ESP+16]
		MOV		EBX,[ESP+20]
		MOV		[ECX+16],EDX	; 复制传递给hrb_api
		MOV		[ECX+20],EBX	; 复制传递给hrb_api
		MOV		EDX,[ESP+24]
		MOV		EBX,[ESP+28]
		MOV		[ECX+24],EDX	; 复制传递给hrb_api
		MOV		[ECX+28],EBX	; 复制传递给hrb_api

		MOV		ES,AX			; 将剩余的段寄存器设置为系统用
		MOV		SS,AX
		MOV		ESP,ECX
		STI			; 切换完成后恢复中断请求

		CALL	_hrb_api	;系统调用

		MOV		ECX,[ESP+32]	; 取出应用程序ESP
		MOV		EAX,[ESP+36]	; 取出应用程序SS
		CLI						; 禁止中断切换回用户态
		MOV		SS,AX
		MOV		ESP,ECX
		POPAD
		POP		ES
		POP		DS
		IRETD		; 这个命令可以恢复中断
		
_start_app:		; void start_app(int eip, int cs, int esp, int ds);
		PUSHAD		; 将8个通用寄存器的值全部保存起来,
		MOV		EAX,[ESP+36]	; 应用程序用的EIP
		MOV		ECX,[ESP+40]	; 应用程序用的CS
		MOV		EDX,[ESP+44]	; 应用程序用的ESP
		MOV		EBX,[ESP+48]	; 应用程序用的DS/SS
		MOV		[0xfe4],ESP		; 操作系统的ESP，保存在这里，以便返回系统时使用
		CLI			; 切换过程中禁止中断请求
		MOV		ES,BX
		MOV		SS,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
		MOV		ESP,EDX
		STI			; 切换完成后恢复中断请求
		PUSH	ECX				; 用于far-CALL的PUSH（cs）
		PUSH	EAX				; 用于far-CALL的PUSH（eip）
		CALL	FAR [ESP]		; 调用应用程序

;	应用程序结束后返回此处，返回系统态切换

		MOV		EAX,1*8			; OS操作系统DS/SS
		CLI			; 切换过程中禁止中断请求
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]
		STI			; 切换完成后恢复中断请求
		POPAD	; 将8个通用寄存器的值全部恢复
		RET		
		