[INSTRSET "i486p"]
[BITS 32]
		MOV		EAX,1*8			; OS用的段号
		MOV		DS,AX			; 将其存入DS
		MOV		BYTE [0x102600],0	;通过将os段号存入DS,间接改变os数据
		MOV		EDX,4
		INT		0x40
