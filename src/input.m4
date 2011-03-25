define(`check_keyboard',`
* Check for user stop request
KEYCHK	pshs	a
	JSR	[$A000]
	BEQ	KEYCHK1
	cmpa	#03
	beq	EXIT
* Check for pause
	cmpa	#32
	beq	KEYCHK2
KEYCHK1	puls	a
	rts
* Pause until user hits space again
KEYCHK2	jsr	[$A000]
	beq	KEYCHK2
	cmpa	#03
	beq	EXIT
	cmpa	#32
	bne	KEYCHK2
	puls	a
')dnl
