	NAM	CoCoAud
	TTL	Audio player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

RSTVEC	EQU	$0072
IRQADR	EQU	$7F66

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

* 324.625 for 11025 Hz
TIMEVAL	EQU	325

	ORG	LOAD

EXEC	EQU	*

* Disable IRQ and FIRQ
	ORCC	#$50
* Disable PIA0 IRQ generation
	LDX	#$FF00
	LDA	1,X
	ANDA	#$3E
	STA	1,X
	LDA	,X
	LDA	3,X
	ANDA	#$3E
	STA	3,X
	LDA	2,X
* Disable PIA1 FIRQ generation
	LDX	#$FF20
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

* Setup palette...
	LDX	#RPALINI
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDRPIN
	BNE	PINTLOP

* ...clear screen...
	LDX	#$2000
	CLRA
CLRSCN	STA	,X+
* Check for $3840 to account for stray half-line at bottom of screen
	CMPX	#$3840
	BNE	CLRSCN

* ...and setup video mode
	LDX	#VIDINIT
	LDY	#VIDOFF
VINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDVINT
	BNE	VINTLOP

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
	LDA	$FF93
	ORA	#$20
	STA	$FF93
	LDA	$FF90
	ORA	#$10
	STA	$FF90

* Init IDE drive
	ldx	#$ff50
* Start w/ recalibrate
	clr	1,x
	clr	2,x
	clr	3,x
	clr	4,x
	clr	5,x
	lda	#$a0
	sta	6,x
	lda	#$10	
	sta	7,x
iderec1	lda	7,x
	anda	#$80
	bne	iderec1
* Assume it worked -- what else would we do?

* Now issue first read command
	clr	1,x
	lda	#$01
	sta	2,x
	clr	3,x
	clr	4,x
	clr	5,x
	lda	#$e0
	sta	6,x
	lda	#$20	
	sta	7,x
ideread	lda	7,x
	anda	#$80
	bne	ideread

* Enable IRQ handling
	ANDCC	#$EF

* Init UART pointer
	ldy	#$ff68

INLOOP	ldb	$ff93
	beq	INLOOP1
	bsr	SNDPOLL
* Check UART for activity
INLOOP1	lda	1,y
	bita	#$08
	bne	INLOOP2
	BRA	INLOOP
INLOOP2	lda	,y
* fall-through to EXIT

* Do some minimal cleanup and reset
EXIT	lda	#$cc
	sta	$ff90
	clr	$ff91
	clr	$ff92
	clr	$ff93
	JMP	[RSTVEC]

* Read two samples and stuff them into the DAC, waiting for timer in between
SNDPOLL	lda	,x
	sta	$ff20
	sta	$ff7a
	sta	$ff7b
SNDWAI1 ldb	$ff93
	beq	SNDWAI1
* Read and store high 8-bits
	lda	8,x
	sta	$ff20
	sta	$ff7a
	sta	$ff7b
* Check if more data available for next round
	lda	7,x
	anda	#$08
	beq	SNDDREQ
	rts
* Increment LBA and request next sector
SNDDREQ
	lda	#$01
	sta	2,x
	inc	3,x
	bne	SNDRCMD
	inc	4,x
	bne	SNDRCMD
	inc	5,x
	bne	SNDRCMD
	lda	6,x
	inca
	anda	#$0f
	ora	#$e0
	sta	6,x
SNDRCMD lda	#$20
	sta	7,x
SNDWAI2 lda	7,x
	anda	#$80
	bne	SNDWAI2
	rts

* Init for video mode, set video buffer to $2000
* (Assumes default MMU setup...)
VIDINIT	FCB	$4C,$00,$00,$00,$00,$00,$00,$00
	FCB	$82,$12,$00,$00,$0F,$E4,$00,$00
ENDVINT	EQU	*

* Init for palette regs
RPALINI	FCB	$00,$08,$10,$18,$20,$28,$30,$38
	FCB	$07,$0F,$17,$1F,$27,$2F,$37,$3F
ENDRPIN	EQU	*
CPALINI	FCB	$00,$1C,$22,$1E,$17,$19,$14,$20
	FCB	$10,$2B,$32,$3D,$36,$38,$34,$30
ENDCPIN	EQU	*

	END	EXEC
