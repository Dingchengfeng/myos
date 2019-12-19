; haribote-os
; TAB=4

[INSTRSET "i486p"]				; 想要使用486指令的叙述

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit 彩色
; （画面模式一览）
;	0x100 :  640 x  400 x 8bit彩色
;	0x101 :  640 x  480 x 8bit彩色
;	0x103 :  800 x  600 x 8bit彩色
;	0x105 : 1024 x  768 x 8bit彩色
;	0x107 : 1280 x 1024 x 8bit彩色

BOTPAK	EQU		0x00280000		; bootpack加载目标
DSKCAC	EQU		0x00100000		; 磁盘缓存位置
DSKCAC0	EQU		0x00008000		; 磁盘缓存位置（实模式）

; BOOT_INFO相关
CYLS	EQU		0x0ff0		;设定启动区
LEDS	EQU		0x0ff1		;键盘指示灯状态
VMODE	EQU		0x0ff2		;关于颜色数目的信息。颜色位数。
SCRNX	EQU		0x0ff4		;分辨率的X
SCRNY	EQU		0x0ff6		;分辨率的Y
VRAM	EQU		0x0ff8		;图像缓冲区的开始地址

		ORG		0xc200		;这个程序会被装载到内存的这个位置
		
; 确认VBE是否存在

		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0		;如果能用的话，ES:DI开始的512字节会写入此显卡能用的VBE信息
		MOV		AX,0x4f00
		INT		0x10		;以上变量初始化后调0x10,如果有VBE的话，AX就会变为0x004f
		CMP		AX,0x004f
		JNE		scrn320
		
; VBE版本检查

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320
		
; 判断VBEMODE所指模式能否使用，取得画面模式信息，存入ES:DI开始的256字节中

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320
		
; 画面模式信息的确认

		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]		; 模式属性的bit7是0就放弃，bit7判断画面模式是否可以加上0x4000再进行指定
		AND		AX,0x0080
		JZ		scrn320	

; 画面模式的切换

		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus	

scrn320:
		MOV		AL,0x13			; VGA图、320x200x8bit彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000		
	
;用BIOS取得键盘上各种LED指示灯的状态

keystatus:
		MOV		AH,0x02
		INT		0x16		;键盘BIOS
		MOV		[LEDS],AL	
		
; PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC
;	必须在CLI之前进行，否则有时会挂起。
;	随后进行PIC的初始化。

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 如果连续使用OUT命令，有些机种会无法正常运行
		OUT		0xa1,AL

		CLI						; 禁止CPU级别的中断

; 为了让CPU能够访问1M以上的内存空间，设定A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式

		LGDT	[GDTR0]			; 设定临时的GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设bit31为0（为了禁止分页）
		OR		EAX,0x00000001	; 设bit0为1（为了切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; 可读写的段 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的转送

		MOV		ESI,bootpack	; 转送源
		MOV		EDI,BOTPAK		; 转送目的地
		MOV		ECX,512*1024/4
		CALL	memcpy

; 磁盘数据最终转送到它本来的位置去

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 转送源
		MOV		EDI,DSKCAC		; 转送目的地
		MOV		ECX,512/4
		CALL	memcpy

; 所有剩下的

		MOV		ESI,DSKCAC0+512	; 转送源
		MOV		EDI,DSKCAC+512	; 转送目的地
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy

; 必须由asmhead来完成的工作，至此全部完毕
;	以后就交由bootpack来完成

; bootpack的启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要转送的东西
		MOV		ESI,[EBX+20]	; 转送源
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转送目的地
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND的结果如果不是0，就跳转到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; SUB 的结果如果不是0，就跳转到memcpy
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

		ALIGNB	16
GDT0:
		RESB	8				; NULL selector(8字节的0)
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写的段（segment）32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行的段 32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
