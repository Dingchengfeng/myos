[BITS 32]
		MOV		AL,'h'
		INT		0x40	; _asm_cons_putchar函数的中断向量号
		MOV		AL,'e'
		INT		0x40
		MOV		AL,'l'
		INT		0x40
		MOV		AL,'l'
		INT		0x40
		MOV		AL,'o'
		INT		0x40
		RETF
