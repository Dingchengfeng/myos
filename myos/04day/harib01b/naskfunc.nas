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

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4]���I?��addr;ESP��?�w?�񑶊�
		MOV		AL,[ESP+8]		; [ESP+8]���I?��data
		MOV		[ECX],AL
		RET
