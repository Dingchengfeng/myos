; hello-os
; TAB=4		
		ORG		0x7c00		;bios会默认把启动程序装载到这个地址，这是条伪指令，告诉汇编器程序会从内存地址的这个位置执行，后续编译时以这个地址为参照这条指令下，
;标准FAT12格式软盘专用代码

		DB		0xeb, 0x4e, 0x90
		DB		"HELLOIPL"		;启动扇区的名字，可以是任意字符（必须8字节）
		DW		512				;每个扇区大小（必须为512字节）
		DB		1				;簇的大小（必须为1个扇区）
		DW		1				;FAT的起始位置（一般从第1个扇区开始）
		DB		2				;FAT的个数（必须为2）
		DW		224				;根目录的大小（一般设置成224项）
		DW		2880			;该磁盘的大小（必须是2880扇区）
		DB		0xf0			;磁盘的种类（必须是0xf0）
		DW		9				;FAT的长度（必须是9扇区）
		DW		18				;1个磁道有几个扇区（必须是18）
		DW		2				;磁头数（必须是2）
		DD		0				;是否使用分区（0表示否，必须是0）
		DD		2880			;重写一次磁盘大小
		DB		0,0,0x29		;这三个字节意义不明，必须如此
		DD		0xffffffff		;可能是卷标号码
		DB		"HELLO-OS   "	;磁盘名称（11字节任意字符）
		DB		"FAT12   "		;磁盘格式名称（8字节）
		RESB	18				;空出18字节（这18个字节会用0填充）	

;程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX     ; 栈段寄存器
		MOV		SP,0x7c00 ; 栈指针寄存器
		MOV		DS,AX     ; 数据段寄存器
		MOV		ES,AX			; 附加段寄存器

		MOV		SI,msg		; 源变址寄存器		
putloop:
		MOV		AL,[SI]		; 要显示的文字 [a]表示取地址编号为a的内存地址的值
		ADD		SI,1			; SI里的值自增
		CMP		AL,0			; msg末尾是0
		JE		fin
		MOV		AH,0x0e		; INT 0x10 显示文字设置
		MOV		BX,15			; 指定字符颜色（0-255）(BH=0,BL=color code)
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						  ; 让CPU停止，等待指令,cpu睡眠，任何动作都会唤醒cpu,比如移动鼠标，按下键盘等
		JMP		fin				; 无限循环

msg:
		DB		0x0a, 0x0a		; 2个换行
		DB		"hello, world"
		DB		0x0a			; 换行
		DB		0
		
		RESB	0x7dfe-$		; 填充0x00到0x7dfe（$表示将要读入的内存地址）
		DB		0x55, 0xaa	;启动区末尾
; 启动区以外部分的输出

		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432
