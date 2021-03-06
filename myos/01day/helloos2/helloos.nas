; hello-os
; TAB=4

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

		DB		0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
		DB		0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
		DB		0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
		DB		0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
		DB		0xee, 0xf4, 0xeb, 0xfd

; 信息显示部分

		DB		0x0a, 0x0a		; 2个换行
		DB		"hello, world"
		DB		0x0a			; 换行
		DB		0

		RESB	0x1fe-$			; 填充0x00到0x1fe,这里$符表示当前输出的字节数

		DB		0x55, 0xaa	;启动区末尾标识，如果第一扇区末尾没有这个表示，计算机就不认为这是系统盘

; 启动区以外部分的输出

		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432
