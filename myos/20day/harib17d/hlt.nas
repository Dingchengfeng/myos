[BITS 32]
		MOV		AL,'h'
		CALL    2*8:0xbe8	; 编译后的_asm_cons_putchar函数地址
		MOV		AL,'e'
		CALL    2*8:0xbe8
		MOV		AL,'l'
		CALL    2*8:0xbe8
		MOV		AL,'l'
		CALL    2*8:0xbe8
		MOV		AL,'o'
		CALL    2*8:0xbe8
		RETF
