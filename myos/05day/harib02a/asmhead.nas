; haribote-os
; TAB=4

BOTPAK	EQU		0x00280000		; bootpack加载目标
DSKCAC	EQU		0x00100000		; 磁盘缓存位置
DSKCAC0	EQU		0x00008000		; 磁盘缓存位置（实模式）

;有关BOOT_INFO 这些都是内存地址
CYLS	EQU		0x0ff0		;柱面数
LEDS	EQU		0x0ff1		;键盘指示灯状态
VMODE	EQU		0x0ff2		;关于颜色数目的信息。颜色位数。0x0ff3暂时没有使用 
SCRNX	EQU		0x0ff4		;分辨率的X
SCRNY	EQU		0x0ff6		;分辨率的Y
VRAM	EQU		0x0ff8		;图像缓冲区的开始地址

		ORG		0xc200		;这个程序会被装载到内存的这个位置

;设置屏幕模式
		
		MOV		AL,0x13		;VGA显卡，320*200*8位彩色(64000B)
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	;记录画面模式，这种模式使用0xa0000-0xaffff 64KB内存(65536B)
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
		
;用BIOS取得键盘上各种LED指示灯的状态

		MOV		AH,0x02
		INT		0x16		;键盘BIOS
		MOV		[LEDS],AL	
		
;   PIC关闭一切中断（CPU进行模式转换时无法处理中断，而且后面进行PIC初始化时也不允许中断）
;	根据AT兼容机的规格，如果要初始化PIC
;	必须在CLI之前进行，否则有时会挂起。
;	随后进行PIC的初始化。

		MOV		AL,0xff
		OUT		0x21,AL			; 禁止主PIC全部中断
		NOP						; 如果连续使用OUT命令，有些机种会无法正常运行
		OUT		0xa1,AL			; 禁止从PIC全部中断

		CLI						; 禁止CPU级别的中断

; 为了让CPU能够访问1M以上的内存空间，设定A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL			
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20 0xdf输出到0x60端口会激活A20GATE信号线，是所有地址线可用，而不是16位模式下的20根地址线
		OUT		0x60,AL			; 往键盘控制电路的附属端口输出0xdf,这个附属端口连接着主板的很多地方，通过这个端口发送不同的指令，就可以实现各种各样的控制功能。
		CALL	waitkbdout		; 这句话是为了等待指令执行完成

; 切换到保护模式

[INSTRSET "i486p"]				; 想要使用486指令的叙述

		LGDT	[GDTR0]			; 设定临时的GDT 将源操作数中的值加载到全局描述符表格寄存器 (GDTR) 源操作数指定 6 字节内存位置
		MOV		EAX,CR0			; 通过设定CR0来切换到保护模式
		AND		EAX,0x7fffffff	; 设bit31为0（为了禁止分页）
		OR		EAX,0x00000001	; 设bit0为1（为了切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush	; 切换到保护模式后，之前流水线上的指令要重新解释，故用JMP破坏流水线
pipelineflush:
		MOV		AX,1*8			; 可读写的段 32bit  进入保护模式后，段寄存器的值表示描述符表的索引，该索引位置存储段基地址，0x0008是第二个索引位置
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

; 必须由asmhead来完成的工作
; 至此全部完毕
; 以后就交由bootpack来完成

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
		RESB	8				; NULL selector(8字节的0),0号是空区域，不能再这里定义段
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写的段（segment）32bit 1号段
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行的段 32bit（bootpack用） 2号段

		DW		0
GDTR0:
		DW		8*3-1		;16位段上限
		DD		GDT0		;32位段起始地址

		ALIGNB	16 ;补0，直到内存是16的倍数
bootpack:
