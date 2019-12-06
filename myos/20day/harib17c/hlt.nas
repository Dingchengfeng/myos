[BITS 32]
		MOV		AL,'A'
		CALL    2*8:0xbe3	; 编译后的_asm_cons_putchar函数地址
fin:
	HLT
	JMP fin
