; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �����?�����I�͎�	
[INSTRSET "i486p"]				; ��???��������intel486?�ʁC
[BITS 32]						; ��???�������32�ʖ͎�
[FILE "naskfunc.nas"]			; ��������������

		GLOBAL	_io_hlt,_write_mem8	;�����S�ǔ�������

[SECTION .text]					;�������e�i�B[SECTION .data]�\���������e�i	
 
_io_hlt:	; void io_hlt(void);
		HLT
		RET

