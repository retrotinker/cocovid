	NAM	CoCoAud
	TTL	Compressed audio player for CoCo3 

LOAD	EQU	$0E00		Actual load address for binary

RSTVEC	EQU	$0072
IRQADR	EQU	$7F66

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

* 324.625 for 11025 Hz
TIMEVAL	EQU	162

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
*	ANDCC	#$EF

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

* Read four samples and stuff them into the DAC, waiting for timer in between
SNDPOLL	lda	,x
	bmi	SNDPOL1
	ldy	#CLWLOTB
	bra	SNDPOL2
SNDPOL1	ldy	#CLWHITB
	anda	#$7f
SNDPOL2	ldd	a,y
	pshs	b
	sta	$ff20
	sta	$ff7a
	sta	$ff7b
SNDWAI1 ldb	$ff93
	beq	SNDWAI1
	puls	a
	sta	$ff20
	sta	$ff7a
	sta	$ff7b
SNDWAI2 ldb	$ff93
	beq	SNDWAI2
* Read and store high 8-bits
	lda	8,x
	bmi	SNDPOL3
	ldy	#CLWLOTB
	bra	SNDPOL4
SNDPOL3	ldy	#CLWHITB
	anda	#$7f
SNDPOL4	ldd	a,y
	pshs	b
	sta	$ff20
	sta	$ff7a
	sta	$ff7b
SNDWAI3 ldb	$ff93
	beq	SNDWAI3
	puls	a
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
SNDWAI4 lda	7,x
	anda	#$80
	bne	SNDWAI4
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

* Sequence: -32,-21,-13, -9, -6, -4, -2, -1,  0,  1,  3,  5,  8, 12, 20, 31
* Unsigned:   0, 11, 19, 23, 26, 28, 30, 31, 32, 33, 35, 37, 40, 44, 52, 63
*  Encoded:  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0a, 0b, 0c, 0d, 0e, 0f
*  Decoded:  00, 0b, 13, 17, 1a, 1c, 1e, 1f, 20, 21, 23, 25, 28, 2c, 34, 3f
*  Shifted:  00, 2c, 4c, 5c, 68, 70, 78, 7c, 80, 84, 8c, 94, a0, b0, d0, fc

CLWLOTB	fdb	$0000, $002c, $004c, $005c, $0068, $0070, $0078, $007c,
	fdb	$0080, $0084, $008c, $0094, $00a0, $00b0, $00d0, $00fc,
	fdb	$2c00, $2c2c, $2c4c, $2c5c, $2c68, $2c70, $2c78, $2c7c,
	fdb	$2c80, $2c84, $2c8c, $2c94, $2ca0, $2cb0, $2cd0, $2cfc,
	fdb	$4c00, $4c2c, $4c4c, $4c5c, $4c68, $4c70, $4c78, $4c7c,
	fdb	$4c80, $4c84, $4c8c, $4c94, $4ca0, $4cb0, $4cd0, $4cfc,
	fdb	$5c00, $5c2c, $5c4c, $5c5c, $5c68, $5c70, $5c78, $5c7c,
	fdb	$5c80, $5c84, $5c8c, $5c94, $5ca0, $5cb0, $5cd0, $5cfc,
	fdb	$6800, $682c, $684c, $685c, $6868, $6870, $6878, $687c,
	fdb	$6880, $6884, $688c, $6894, $68a0, $68b0, $68d0, $68fc,
	fdb	$7000, $702c, $704c, $705c, $7068, $7070, $7078, $707c,
	fdb	$7080, $7084, $708c, $7094, $70a0, $70b0, $70d0, $70fc,
	fdb	$7800, $782c, $784c, $785c, $7868, $7870, $7878, $787c,
	fdb	$7880, $7884, $788c, $7894, $78a0, $78b0, $78d0, $78fc,
	fdb	$7c00, $7c2c, $7c4c, $7c5c, $7c68, $7c70, $7c78, $7c7c,
	fdb	$7c80, $7c84, $7c8c, $7c94, $7ca0, $7cb0, $7cd0, $7cfc,

CLWHITB	fdb	$8000, $802c, $804c, $805c, $8068, $8070, $8078, $807c,
	fdb	$8080, $8084, $808c, $8094, $80a0, $80b0, $80d0, $80fc,
	fdb	$8400, $842c, $844c, $845c, $8468, $8470, $8478, $847c,
	fdb	$8480, $8484, $848c, $8494, $84a0, $84b0, $84d0, $84fc,
	fdb	$8c00, $8c2c, $8c4c, $8c5c, $8c68, $8c70, $8c78, $8c7c,
	fdb	$8c80, $8c84, $8c8c, $8c94, $8ca0, $8cb0, $8cd0, $8cfc,
	fdb	$9400, $942c, $944c, $945c, $9468, $9470, $9478, $947c,
	fdb	$9480, $9484, $948c, $9494, $94a0, $94b0, $94d0, $94fc,
	fdb	$a000, $a02c, $a04c, $a05c, $a068, $a070, $a078, $a07c,
	fdb	$a080, $a084, $a08c, $a094, $a0a0, $a0b0, $a0d0, $a0fc,
	fdb	$b000, $b02c, $b04c, $b05c, $b068, $b070, $b078, $b07c,
	fdb	$b080, $b084, $b08c, $b094, $b0a0, $b0b0, $b0d0, $b0fc,
	fdb	$d000, $d02c, $d04c, $d05c, $d068, $d070, $d078, $d07c,
	fdb	$d080, $d084, $d08c, $d094, $d0a0, $d0b0, $d0d0, $d0fc,
	fdb	$fc00, $fc2c, $fc4c, $fc5c, $fc68, $fc70, $fc78, $fc7c,
	fdb	$fc80, $fc84, $fc8c, $fc94, $fca0, $fcb0, $fcd0, $fcfc,

	END	EXEC
