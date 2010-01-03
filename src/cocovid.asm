	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

RSTVEC	EQU	$0072
IRQADR	EQU	$010d

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

AUBUFSZ	EQU	$02df

* Frame step value should be 2x actual frame step for 30fps source video
FRAMSTP	EQU	4

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
	LDX	#CPALINI
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDCPIN
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

* Clear audio buffer
	LDX	#$3800
	CLRA
CLRAUD	STA	,X+
	CMPX	#$4800
	BNE	CLRAUD

* Init audio buffer switching and video frame step
	LDA	#FRAMSTP
	STA	>STEPCNT
	CLR	>FRAMCNT
	LDD	#$4000
	TFR	D,Y
	ADDD	#AUBUFSZ
	STD	>AUDRSTP
	LDD	#$3800
	STD	>AUDWPTR
	ADDD	#AUBUFSZ
	STD	>AUDWSTP

* Init Vsync interrupt generation
	LDA	$FF92
	ORA	#$08
	STA	$FF92
	LDA	$FF90
	ORA	#$20
	STA	$FF90

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
	ldu	#$ff50
	clrb
* Clear pending data transfers
ideclr	lda	7,u
	anda	#$08
	beq	idecal
	lda	,u
	lda	8,u
	bra	ideclr
* Issue recalibrate command
idecal	clr	1,u
	clr	2,u
	clr	3,u
	clr	4,u
	clr	5,u
	lda	#$a0
	sta	6,u
	lda	#$10	
	sta	7,u
iderec1	lda	7,u
	anda	#$80
	bne	iderec1
* Assume it worked -- what else would we do?

* Now issue first read command
	clr	1,u
	lda	#$80
	sta	2,u
	clr	3,u
	clr	4,u
	clr	5,u
	lda	#$e0
	sta	6,u
	lda	#$20	
	sta	7,u
ideread	lda	7,u
	anda	#$80
	bne	ideread

* Data movement goes here
INLOOP	LDX	#$2000
* Check timer interrupt
INLOP1	LDA	$FF93
	BEQ	INLOP2
* Load and play sample
	LDA	,Y+
	sta	$ff20
	sta	$ff7a
	CMPY	>AUDRSTP
	BLT	INLOP2
	LEAY	-1,Y
INLOP2	lda	$ff92
	beq	INLOP3
	lbsr	VIDTMR
INLOP3	lbsr	DATREAD
* Check for escape char
	pshs	a
	anda	#$C0
	cmpa	#$C0
	LBEQ	PIXESC
	puls	a
* Store data in video buffer
	STA	,X+
	BRA	INLOP1

* Audio data movement goes here
* Check timer interrupt
AUDIN	LDX	>AUDWPTR
AUDIN1	LDA	$FF93
	BEQ	AUDIN3
* Load and play sample
	LDA	,Y+
	sta	$ff20
	sta	$ff7a
	CMPY	>AUDRSTP
	BLT	AUDIN2
	LEAY	-1,Y
AUDIN2	lda	$ff92
	beq	AUDIN3
	lbsr	VIDTMR
AUDIN3	lbsr	DATREAD
* Store data in audio buffer
	STA	,X+
	CMPX	>AUDWSTP
	BLT	AUDIN1

* Synchronize!
* Check timer interrupt
SYNCLOP	LDA	$FF93
	BEQ	SYNCLP1
* Load and play sample
	LDA	,Y+
	sta	$ff20
	sta	$ff7a
	CMPY	>AUDRSTP
	BLT	SYNCLP1
	LEAY	-1,Y
SYNCLP1	lda	$ff92
	beq	SYNCLP2
	lbsr	VIDTMR
SYNCLP2 LDA	>FRAMCNT
	BEQ	SYNCLOP
	DEC	>FRAMCNT
* Switch to next audio frame
	pshs	b
	TFR	Y,D
	ANDA	#$78
	CLRB
	STD	>AUDWPTR
	ADDD	#AUBUFSZ
	STD	>AUDWSTP
	ANDA	#$78
	EORA	#$78
	CLRB
	TFR	D,Y
	ADDD	#AUBUFSZ
	STD	>AUDRSTP
	puls	b
	LBRA	INLOOP

* Execute reset vector
EXIT	clr	$ff90
	clr	$ff91
	clr	$ff92
	clr	$ff93
	clr	$ff94
	clr	$ff95
	clr	$ff96
	clr	$ff97
	clr	$ff98
	clr	$ff99
	clr	$ff9a
	clr	$ff9b
	clr	$ff9c
	clr	$ff9d
	clr	$ff9e
	clr	$ff9f
	JMP	[RSTVEC]

PIXESC	puls	a
	CMPA	#$C0
	BNE	PIXMWR
PIXJMP	LDX	#$2000
* Check timer interrupt
PIXJMP1	LDA	$FF93
	BEQ	PIXJMP2
* Load and play sample
	LDA	,Y+
	sta	$ff20
	sta	$ff7a
	CMPY	>AUDRSTP
	BLT	PIXJMP2
	LEAY	-1,Y
PIXJMP2	bsr	DATREAD
	pshs	a
	bsr	DATREAD
	pshs	b
	tfr	a,b
	lda	1,s
	CMPD	#$FFFF
	beq	PIXJMP4
PIXJMP3	LEAX	D,X
	puls	b
	leas	1,s
	JMP	INLOP1
PIXJMP4	puls	b
	leas	1,s
	lbra	AUDIN

PIXMWR	anda	#$3f
	pshs	a
	bsr	DATREAD
* Check timer interrupt
PIXMWR2	TST	$FF93
	BEQ	PIXMWR4
	PSHS	A
* Load and play sample
	LDA	,Y+
	sta	$ff20
	sta	$ff7a
	CMPY	>AUDRSTP
	BLT	PIXMWR3
	LEAY	-1,Y
PIXMWR3	PULS	A
PIXMWR4	STA	,X+
	dec	,s
	BNE	PIXMWR2
	leas	1,s
	JMP	INLOP1

DATREAD	tstb
	bne	DATNEXT
	ldb	#$08
	lda	,u
	rts
DATNEXT lda	8,u
* Check if more data available for next round
	bitb	7,u
	beq	DATREQ
	clrb
	rts
* Increment LBA and request next sector
DATREQ	ldb	#$80
	stb	2,u
	addb	3,u
	stb	3,u
	bne	DATCMD
	inc	4,u
	bne	DATCMD
	inc	5,u
	bne	DATCMD
	ldb	6,u
	incb
	andb	#$0f
	orb	#$e0
	stb	6,u
DATCMD	ldb	#$20
	stb	7,u
DATWAIT ldb	7,u
	andb	#$80
	bne	DATWAIT
* Check UART for activity
DATWAI1	ldb	$ff69
	bitb	#$08
	beq	DATWAI2
	ldb	$ff68
	lbra	EXIT
* Check for user stop request
DATWAI2	pshs	a
	JSR	[$A000]
	BEQ	DATWAI3
	cmpa	#$03
	LBEQ	EXIT
DATWAI3	puls	a
	clrb
	rts

* Clear Vsync interrupt
VIDTMR	pshs	a
* Account for frame timing
	DEC	>STEPCNT
	BNE	VIDTMR2
* Unblock data pump -- limit FRAMCNT to prevent catch-up silliness
	LDA	>FRAMCNT
	INCA
	ANDA	#$07
	STA	>FRAMCNT
* Reset frame skip count
	LDA	#FRAMSTP
	STA	>STEPCNT
VIDTMR2 puls	a
	RTS

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

DATFLAG	FCB	$00

STEPCNT	RMB	1
FRAMCNT	RMB	1

AUDRSTP	RMB	2
AUDWPTR	RMB	2
AUDWSTP	RMB	2

	END	EXEC
