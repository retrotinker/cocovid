	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

IRQADR	EQU	$5F66
VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

* TIMEVAL	EQU	262
TIMEVAL	EQU	1575
* Frame step value should be 2x actual frame step for 30fps source video
FRAMSTP	EQU	1
* Vsync timing seems to be wrong in MESS...
* FRAMSTP	EQU	3

	ORG	LOAD

EXEC	EQU	*

* Disable IRQ and FIRQ
	ORCC	#$50
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

* Setup video mode...
	LDX	#VIDINIT
	LDY	#VIDOFF
VINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDVINT
	BNE	VINTLOP

* ...and clear screen
	LDX	#$2000
	CLRA
CLRSCN	STA	,X+
* Check for $3840 to account for stray half-line at bottom of screen
	CMPX	#$3840
	BNE	CLRSCN

* Setup palette (move this up?)
	LDX	#PALINIT
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDPINT
	BNE	PINTLOP

* Init frame skip
	LDA	#FRAMSTP
	STA	>STEPCNT

** Init Vsync interrupt generation
*	LDA	$FF92
*	ORA	#$08
*	STA	$FF92
*	LDA	$FF90
*	ORA	#$20
*	STA	$FF90
* Init timer interrupt generation
	LDD	#TIMEVAL
	STD	$FF94
	LDA	$FF91
	ANDA	#$DF
	STA	$FF91
	LDA	$FF92
	ORA	#$20
	STA	$FF92
	LDA	$FF90
	ORA	#$20
	STA	$FF90

* Enable IRQ handling
	LDD	#VIDISR
	STD	IRQADR
	ANDCC	#$EF

INLOOP	JSR	[$A000]
	BEQ	INLOOP
	CMPA	#$03
	BEQ	EXIT
	BRA	INLOOP

EXIT	JMP	[$FFFE]

* Clear Vsync interrupt
VIDISR	LDB	$FF92
* Account for frame timing
	DEC	>STEPCNT
	BNE	VIDISR3
	LDX	#$2000
VIDISR1	LDA	$FFE1
* For now, loop on no char...
	BEQ	VIDISR1
	LDA	$FFE0
* check for escape char
	TFR	A,B
	ANDB	#$C0
	CMPB	#$C0
	BEQ	PIXMWR
	STA	,X+
VIDISR2	CMPX	#$3800
	BNE	VIDISR1
* Reset frame skip count
	LDA	#FRAMSTP
	STA	>STEPCNT
VIDISR3	RTI

PIXMWR	TFR	A,B
	ANDB	#$3F
PIXMWR1	LDA	$FFE1
	BEQ	PIXMWR1
	LDA	$FFE0
PIXMWR2	STA	,X+
	DECB
	BNE	PIXMWR2
	JMP	VIDISR2

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

	END	EXEC
