	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

RSTVEC	EQU	$0072
IRQADR	EQU	$010d

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

AUBUFSZ	EQU	$0170
ABFPTR1	EQU	$1400
ABFPTR2	EQU	$1800
ABUFEND	EQU	$1C00
ABUFMSK	EQU	$1C
ABFEMSK	EQU	$0C

VIDBUF	EQU	$1C00
VBUFEND	EQU	$7C00

* Frame step value should be 2x actual frame step for 30fps source video
FRAMSTP	EQU	2

* 325.1 for 11025 Hz
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

* Setup stack...
*	lds	#$7f60

* Setup palette...
	LDX	#RPALINI
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDRPIN
	BNE	PINTLOP

* ...clear screen...
	LDX	#VIDBUF
	CLRA
CLRSCN	STA	,X+
	CMPX	#VBUFEND
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
	LDX	#ABFPTR1
	CLRA
CLRAUD	STA	,X+
	CMPX	#ABUFEND
	BNE	CLRAUD

* Init audio buffer switching and video frame step
	LDA	#FRAMSTP
	STA	>STEPCNT
	CLR	>FRAMCNT
	LDD	#ABFPTR2
	TFR	D,Y
	ADDD	#AUBUFSZ
	STD	>AUDRSTP
	LDD	#ABFPTR1
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

* Init VHD access
	ldu	#$ff80
	clr	,u
	clr	1,u
	clr	2,u
	ldd	#VHDBUF
	sta	4,u
	stb	5,u
* Read first sector
	clr	3,u
* Check status
	lda	3,u
* Exit on error
	lbne	EXIT
* Point at buffer base
	ldu	#VHDBUF
	clrb
* Clear stored record address
	clr	>VHDLRNH
	clr	>VHDLRNM
	clr	>VHDLRNL

* Data movement goes here
INLOOP	LDX	#VIDBUF
* Check timer interrupt
INLOP1	LDA	$FF93
	BEQ	INLOP2
* Load and play sample
	LDA	,Y+
	sta	$ff20
	CMPY	>AUDRSTP
	BLT	INLOP2
	LEAY	-1,Y
INLOP2	lda	$ff92
	beq	INLOP3
	lbsr	VIDTMR
INLOP3
* Here to DATGT1 is for reading next byte from vhd
	lda	b,u
	incb
	cmpb	#$80
	bne	DATGT1
	lbsr	DATREQ
DATGT1
* Check for escape char
	pshs	a
	anda	#$C0
	cmpa	#$C0
	beq	PIXESC
	puls	a
* Store data in video buffer
	STA	,X+
	BRA	INLOP1

PIXESC	puls	a
	CMPA	#$C0
	BNE	PIXMWR
PIXJMP	LDX	#VIDBUF
* Check timer interrupt
PIXJMP1	LDA	$FF93
	BEQ	PIXJMP2
* Load and play sample
	LDA	,Y+
	sta	$ff20
	CMPY	>AUDRSTP
	BLT	PIXJMP2
	LEAY	-1,Y
PIXJMP2
* Here to DATGT3 is for reading next byte from vhd
	lda	b,u
	incb
	cmpb	#$80
	bne	DATGT3
	bsr	DATREQ
DATGT3
	pshs	a
* Here to DATGT4 is for reading next byte from vhd
	lda	b,u
	incb
	cmpb	#$80
	bne	DATGT4
	bsr	DATREQ
DATGT4
	pshs	b
	tfr	a,b
	lda	1,s
	cmpa	#$ff
	beq	PIXJMP3
	LEAX	D,X
	puls	b
	leas	1,s
	JMP	INLOP1
PIXJMP3	cmpb	#$ff
	lbne	EXIT
PIXJMP4	puls	b
	leas	1,s
	lbra	AUDIN

PIXMWR	anda	#$3f
	pshs	a
* Here to DATGT5 is for reading next byte from vhd
	lda	b,u
	incb
	cmpb	#$80
	bne	DATGT5
	bsr	DATREQ
DATGT5
* Check timer interrupt
PIXMWR2	TST	$FF93
	BEQ	PIXMWR4
	PSHS	A
* Load and play sample
	LDA	,Y+
	sta	$ff20
	CMPY	>AUDRSTP
	BLT	PIXMWR3
	LEAY	-1,Y
PIXMWR3	PULS	A
	tst	$ff92
	beq	PIXMWR4
	bsr	VIDTMR
PIXMWR4	STA	,X+
	dec	,s
	BNE	PIXMWR2
	leas	1,s
	JMP	INLOP1

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

* Increment LBA and request next sector
DATREQ	cmpu	#VHDBFHI
	beq	DATREQ1
	ldu	#VHDBFHI
	clrb
	rts
DATREQ1
	ldu	#$ff80
	inc	>VHDLRNL
	ldb	>VHDLRNL
	stb	2,u
	bne	DATCMD
	inc	>VHDLRNM
	ldb	>VHDLRNM
	stb	1,u
	bne	DATCMD
	inc	>VHDLRNH
	ldb	>VHDLRNH
	stb	,u
DATCMD
	clr	3,u
	ldb	3,u
* Abort on error...ick...
	lbne	EXIT
	ldu	#VHDBUF
	clrb
* Check for user stop request
BRKCHK	pshs	a
	JSR	[$A000]
	BEQ	BRKCHK1
	cmpa	#$03
	lbeq	EXIT
* Check for pause
	cmpa	#$20
	beq	BRKCHK2
BRKCHK1	puls	a
	rts
* Pause until user hits space again
BRKCHK2	jsr	[$A000]
	beq	BRKCHK2
	cmpa	#$20
	bne	BRKCHK2
	puls	a
	rts

* Audio data movement goes here
* Check timer interrupt
AUDIN	LDX	>AUDWPTR
AUDIN1	LDA	$FF93
	BEQ	AUDIN3
* Load and play sample
	LDA	,Y+
	sta	$ff20
	CMPY	>AUDRSTP
	BLT	AUDIN2
	LEAY	-1,Y
AUDIN2	lda	$ff92
	beq	AUDIN3
	lbsr	VIDTMR
AUDIN3
* Here to DATGT2 is for reading next byte from vhd
	lda	b,u
	incb
	cmpb	#$80
	bne	DATGT2
	bsr	DATREQ
DATGT2
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
	ANDA	#ABUFMSK
	CLRB
	STD	>AUDWPTR
	ADDD	#AUBUFSZ
	STD	>AUDWSTP
	ANDA	#ABUFMSK
	EORA	#ABFEMSK
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
	JMP	[$fffe]

* Init for video mode, set video buffer to VIDBUF
* (Assumes default MMU setup...)
VIDINIT	FCB	$4C,$00,$00,$00,$00,$00,$00,$00
	FCB	$80,$12,$00,$00,$0F,$E3,$80,$00
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

VHDLRNH	RMB	1
VHDLRNM	RMB	1
VHDLRNL	RMB	1

VHDBUF	RMB	128
VHDBFHI	RMB	128

	END	EXEC
