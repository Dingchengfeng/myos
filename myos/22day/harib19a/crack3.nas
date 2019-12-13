[INSTRSET "i486p"]
[BITS 32]
		MOV		AL,0x34
		OUT		0x43,AL
		MOV		AL,0xff
		OUT		0x40,AL
		MOV		AL,0xff
		OUT		0x40,AL

; 	上述代码的功能与下面代码相当，应用程序模式运行，io指令会产生一般异常保护
;	io_out8(PIT_CTRL, 0x34);
;	io_out8(PIT_CNT0, 0xff);
;	io_out8(PIT_CNT0, 0xff);

		MOV		EDX,4
		INT		0x40
