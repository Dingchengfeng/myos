; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目?文件的模式	
[INSTRSET "i486p"]				; 告???器程序是基于intel486?写，
[BITS 32]						; 告???器程序是32位模式
[FILE "naskfunc.nas"]			; 程序源文件名称

		GLOBAL	_io_hlt,_write_mem8	;声明全局函数名称

[SECTION .text]					;函数内容段。[SECTION .data]表示数据内容段	
 
_io_hlt:	; void io_hlt(void);
		HLT
		RET

