	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

IRQADR	EQU	$5F66
FIRQADR	EQU	$5F68

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

* Frame step value should be 2x actual frame step for 30fps source video
FRAMSTP	EQU	8

* 324 for 11025Hz...
TIMEVAL	EQU	648

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
	LDX	#PALINIT
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDPINT
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
	LDD	#$4000
	TFR	D,Y
	ADDD	#$02E0
	STD	>AUDRSTP
	LDD	#$3800
	TFR	D,U
	ADDD	#$02E0
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

* Wait for user input to start
STRTWT	JSR	[$A000]
	BEQ	STRTWT
	CLR	>FRAMCNT

* Enable IRQ and FIRQ handling
	LDD	#VIDISR
	STD	IRQADR
	LDD	#SNDISR
	STD	FIRQADR
	ANDCC	#$AF

* Data movement goes here
INLOOP	LDX	#$2000
INLOP1	LDA	$FFE1
* Check for character available
	BEQ	INLOP1
INLOP2	LDA	$FFE0
* Check for escape char
	TFR	A,B
	ANDB	#$C0
	CMPB	#$C0
	BEQ	PIXESC
* Store data in video buffer
	STA	,X+
INLOP3	CMPX	#$3800
	BLT	INLOP1
* Audio data movement goes here
INLOP4	LDA	$FFE1
* Check for character available
	BEQ	INLOP4
INLOP5	LDA	$FFE0
* Store data in audio buffer
	STA	,U+
	CMPU	>AUDWSTP
	BLT	INLOP4
* Synchronize!
SYNCLOP	LDA	>FRAMCNT
	BEQ	SYNCLOP
	DEC	>FRAMCNT
* Switch to next audio frame
	TFR	Y,D
	ANDA	#$78
	CLRB
	TFR	D,U
	ADDD	#$02E0
	STD	>AUDWSTP
	ANDA	#$78
	EORA	#$78
	CLRB
	ORCC	#$40
	TFR	D,Y
	ADDD	#$02E0
	STD	>AUDRSTP
	ANDCC	#$BF
	BRA	INLOOP

* Execute reset vector
EXIT	JMP	[$FFFE]

PIXESC	CMPA	#$C0
	BNE	PIXMWR
PIXJMP	LDX	#$2000
PIXJMP1	LDA	$FFE1
	BEQ	PIXJMP1
	LDA	$FFE0
PIXJMP2	LDB	$FFE1
	BEQ	PIXJMP2
	LDB	$FFE0
	LEAX	D,X
	JMP	INLOP3

PIXMWR	TFR	A,B
	ANDB	#$3F
PIXMWR1	LDA	$FFE1
	BEQ	PIXMWR1
	LDA	$FFE0
PIXMWR2	STA	,X+
	DECB
	BNE	PIXMWR2
	JMP	INLOP3

* Clear Vsync interrupt
VIDISR	LDB	$FF92
* Account for frame timing
	DEC	>STEPCNT
	BNE	VIDISR1
* Reset frame skip count
	LDA	#FRAMSTP
	STA	>STEPCNT
* Unblock data pump
	INC	>FRAMCNT
* Check for user stop request
	JSR	[$A000]
	BEQ	VIDISR1
	CMPA	#$03
	BEQ	EXIT
VIDISR1	RTI

* Read samples and stuff them into the DAC
SNDISR	PSHS	A
* Clear timer interrupt
	LDA	$FF93
* Load and play sample
	LDA	,Y+
	STA	$FF20
	CMPY	>AUDRSTP
	BGE	SNDISR3
SNDISR2 PULS	A
	RTI
SNDISR3 LEAY	-1,Y
	PULS	A
	RTI

* Init for video mode, set video buffer to $2000
* (Assumes default MMU setup...)
VIDINIT	FCB	$4C,$00,$00,$00,$00,$00,$00,$00
	FCB	$82,$12,$00,$00,$0F,$E4,$00,$00
ENDVINT	EQU	*

* Init for palette regs
PALINIT	FCB	$00,$08,$10,$18,$20,$28,$30,$38
	FCB	$07,$0F,$17,$1F,$27,$2F,$37,$3F
ENDPINT	EQU	*

STEPCNT	RMB	1
FRAMCNT	RMB	1

AUDRSTP	RMB	2
AUDWSTP	RMB	2

	END	EXEC
