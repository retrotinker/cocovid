include(`storage.m4')
include(`timers.m4')
include(`input.m4')

	NAM	CoCoVid
	TTL	Video player for CoCo3

LOAD	EQU	$0E00		Actual load address for binary

RSTVEC	EQU	$0072
IRQJMP	EQU	$010c
IRQADR	EQU	$010d
FIRQJMP	EQU	$010f
FIRQADR	EQU	$0110

VIDOFF	EQU	$FF90
PALOFF	EQU	$FFB0
SAMR1ST	EQU	$FFD9

AUBUFSZ	EQU	$02e0
ABFPTR1	EQU	$1400
ABFPTR2	EQU	$1800
ABUFEND	EQU	$1C00
ABUFMSK	EQU	$1C
ABFEMSK	EQU	$0C

VIDBUF	EQU	$1C00
VBUFEND	EQU	$7C00

* Frame step value should be 2x actual frame step for 30fps source video
FRAMSTP	EQU	4

* 325.1 for 11025 Hz
TIMEVAL	EQU	325

	ORG	LOAD

VIDSTRT	FCB	$00,$00,$00,$00

EXEC	EQU	*

* Disable IRQ and FIRQ
	orcc	#$50

* Initialize PIAs
* - Disable PIA[01] interrupt generation
* - Select internal DAC for sound output
* - Enable sound output
	ldx	#$ff00
	lda	#$34
	sta	1,x
	lda	#$35
	sta	3,x
	lda	#$30
	sta	$21,x
	lda	#$fc
	sta	$20,x
	lda	#$34
	sta	$21,x
	lda	#$38
	sta	$23,x
* Bleed-off PIA[01] interrupts
	lda	,x
	lda	2,x
	lda	$20,x
	lda	$22,x

* High-speed poke...definitely...
	sta	SAMR1ST

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
	init_video_timer

* Init (audio) timer interrupt generation
	init_audio_timer

* Init storage access
	init_storage

* Enable Vsync and (audio) timer interrupts
	andcc	#$af

* Data movement goes here
VIDFRM	ldx	#VIDBUF
VIDLOOP	data_read
* Check for escape char
	pshs	a
	anda	#$f0
	cmpa	#$f0
	beq	PIXESC
	puls	a
* Store data in video buffer
	STA	,X+
	bra	VIDLOOP

PIXESC	puls	a
	CMPA	#$f0
	BNE	PIXMWR
PIXJMP	LDX	#VIDBUF
	data_read
	pshs	a
	data_read
	pshs	b
	tfr	a,b
	lda	1,s
	cmpa	#$ff
	beq	PIXJMP3
	LEAX	D,X
	puls	b
	leas	1,s
	jmp	VIDLOOP
PIXJMP3	cmpb	#$ff
	lbne	EXIT
PIXJMP4	puls	b
	leas	1,s
	bra	AUDFRM

PIXMWR	anda	#$0f
	pshs	a
	data_read
PIXMWR2 sta	,x+
	dec	,s
	BNE	PIXMWR2
	leas	1,s
	jmp	VIDLOOP

* Audio data movement goes here
AUDFRM	ldx	>AUDWPTR
AUDLOOP	data_read
* Store data in audio buffer
	STA	,X+
	CMPX	>AUDWSTP
	BLT	AUDLOOP

* Synchronize!
SYNCLP	LDA	>FRAMCNT
	BEQ	SYNCLP
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
	LBRA	VIDFRM

* Execute reset vector
* (Most of these clr's are unnecessary...?)
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

* Instantiate body of storage driver
	storage_handler

* Handle Vsync interrupt
	video_timer_handler

* Handle (audio) timer interrupt
	audio_timer_handler

* Init for video mode, set video buffer to VIDBUF
* (Assumes default MMU setup...)
VIDINIT	FCB	$4C,$00,$00,$00,$00,$00,$00,$00
	FCB	$80,$1A,$00,$00,$0F,$E3,$80,$00
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

* Allocate any variables used by storage driver
	storage_variables

	END	EXEC
