	NAM	CoCoAud
	TTL	Audio player for CoCo

LOAD	EQU	$0E00		Actual load address for binary

IRQADR	EQU	$5F66
SAMR1ST	EQU	$FFD9

TIMEVAL	EQU	324

	ORG	LOAD

* Disable IRQ and FIRQ
EXEC	ORCC	#$50
* Disable PIA IRQ generation
	LDX	#$FF00
	LDA	1,X
	ANDA	#$3E
	STA	1,X
	LDA	,X
	LDA	3,X
	ANDA	#$3E
	STA	3,X
	LDA	2,X

* High-speed poke...definitely...
	STA	SAMR1ST

* Select DAC sound output
	LDA	$FF01
	ANDA	#$C7
	ORA	#$30
	STA	$FF01
	LDA	$FF03
	ANDA	#$C7
	ORA	#$30
	STA	$FF03
* Enable sound output
	LDA	$FF23
	ANDA	#$C7
	ORA	#$38
	STA	$FF23
* Init timer interrupt generation
	LDD	#TIMEVAL
	STD	$FF94
	LDA	$FF91
	ORA	#$20
	STA	$FF91
	LDA	$FF92
	ORA	#$20
	STA	$FF92
	LDA	$FF90
	ORA	#$20
	STA	$FF90
* Enable IRQ handling
	LDD	#SNDISR
	STD	IRQADR
	ANDCC	#$EF

INLOOP	JSR	[$A000]
	BEQ	INLOOP
	CMPA	#$03
	BEQ	EXIT
	BRA	INLOOP

EXIT	JMP	[$FFFE]

* Read samples and stuff them into the DAC
SNDISR	LDX	#$FFE0
	LDA	1,X
	BEQ	SNDISR1
	LDA	,X
	STA	$FF20
* Clear timer interrupt
SNDISR1	LDB	$FF92
	RTI

	END	EXEC
