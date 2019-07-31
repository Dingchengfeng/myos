; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式
[INSTRSET "i486p"]				; 表示要使用486的命令	
[BITS 32]						; 制作32位模式的机器语言


; 制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt,_write_mem8			; 程序中包含的函数名


; 以下是实际的函数

[SECTION .text]		; 函数部分开始

_io_hlt:	; void io_hlt(void)
		HLT
		RET
