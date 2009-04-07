	NAM	CoCoAud
	TTL	Audio player for CoCo

LOAD	EQU	$0E00		Actual load address for binary
IRQADR	EQU	$5F66

LINESKP	EQU	$01

	ORG	LOAD

* Disable IRQ and FIRQ
	ORCC	#$50

* Select DAC sound output
EXEC	LDA	$FF01
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
* Init horizontal sync interrupt generation
	LDA	$FF01
	ANDA	#$FC
	ORA	#$01
	STA	$FF01
* Setup line skip
	LDA	#LINESKP
	STA	>SKIPCNT
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
SNDISR	DEC	>SKIPCNT
	BNE	SNDISR1
	LDX	#$FFE0
	LDA	1,X
	BEQ	SNDISR1
	LDA	,X
	STA	$FF20
* Reset line skip
	LDA	#LINESKP
	STA	>SKIPCNT
* Clear horizontal sync pulse interrupt
SNDISR1	LDB	$FF00
	RTI

SKIPCNT	RMB	1

	END	EXEC
