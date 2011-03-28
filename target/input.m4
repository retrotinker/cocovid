define(`check_keyboard',`
* Check for user stop request
KEYCHK	pshs	a
	jsr	[$A000]
	beq	KEYCHK1
	cmpa	#03
	beq	EXIT
* Check for pause
	cmpa	#32
	beq	KEYCHK2
KEYCHK1	puls	a
	bra	KEYCHK4
* Disable IRQ and FIRQ
KEYCHK2	orcc	#80
* Pause until user hits space again
KEYCHK3	jsr	[$A000]
	beq	KEYCHK3
	cmpa	#03
	beq	EXIT
	cmpa	#32
	bne	KEYCHK3
	puls	a
* Enable IRQ and FIRQ
	andcc	#175
KEYCHK4	equ	*
')dnl
