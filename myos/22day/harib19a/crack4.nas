[INSTRSET "i486p"]
[BITS 32]
		CLI		;应用程序模式运行，CLI、STI、HLT这些指令都会产生异常
fin:
		HLT
		JMP		fin
