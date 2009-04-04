*
* This is proof-of-concept code, designed to be run on MESS with my
* pseudotty port feeding raw uncompressed image data in a constant
* stream.  S-l-o-w, but working surprisingly well...
*
	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0

	ORG	LOAD

* Init for video mode, set video buffer to $2000
* (Assumes default MMU setup...)
VIDINIT	FCB	$4C,$00,$00,$00,$00,$00,$00,$00
	FCB	$81,$12,$00,$00,$0F,$E4,$00,$00
ENDVINT	EQU	*

* Init for palette regs
PALINIT	FCB	$00,$08,$10,$18,$20,$28,$30,$38
	FCB	$07,$0F,$17,$1F,$27,$2F,$37,$3F
ENDPINT	EQU	*

EXEC	EQU	*

	LDX	#VIDINIT
	LDY	#VIDOFF
VINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDVINT
	BNE	VINTLOP

	LDX	#PALINIT
	LDY	#PALOFF
PINTLOP	LDA	,X+
	STA	,Y+
	CMPX	#ENDPINT
	BNE	PINTLOP

PAINT	LDX	#$2000
PAINT1	LDA	$FFE1
	BEQ	PAINT1
	LDA	$FFE0
	STA	$40,X
	STA	,X+
	TFR	X,D
	ANDB	#$3F
	BNE	PAINT2
	LEAX	$40,X
PAINT2	CMPX	#$5000
	BNE	PAINT1
	BRA	PAINT

EXIT	JMP	[$FFFE]

	END	EXEC
