; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; ง์ฺ?ถIอฎ	
[INSTRSET "i486p"]				; ???ํ๖ฅ๎ฐintel486?สC
[BITS 32]						; ???ํ๖ฅ32สอฎ
[FILE "naskfunc.nas"]			; ๖นถผฬ

		GLOBAL	_io_hlt,_write_mem8	;บพSวผฬ

[SECTION .text]					;เeiB[SECTION .data]\ฆเei	
 
_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4]
		MOV		AL,[ESP+8]		; [ESP+8]????
		MOV		[ECX],AL
		RET
