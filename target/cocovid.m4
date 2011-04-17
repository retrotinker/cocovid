	NAM	CoCoVid
	TTL	Video player for CoCo3

*
* Register usage:
*
*	Y is used after init for pointer to next audio sample to play
*	U is used in the VHD driver as pointer to next data to read
*
* All else is fair game!
*

include(`storage.m4')
include(`timers.m4')
include(`input.m4')

*
* This needs to account for code + data (incl. storage driver allocations),
* and it cannot overlap with the video buffer, or ROM, or anything BASIC
* needs if you want to be able to return to it...
*
LOAD	equ	$1700		Actual load address for binary

RSTFLG	equ	$0071

IRQJMP	equ	$010c
IRQADR	equ	$010d
FIRQJMP	equ	$010f
FIRQADR	equ	$0110

PIA0D0	equ	$ff00
PIA0C0	equ	$ff01
PIA0D1	equ	$ff02
PIA0C1	equ	$ff03

PIA1D0	equ	$ff20
PIA1C0	equ	$ff21
PIA1D1	equ	$ff22
PIA1C1	equ	$ff23

GIMECFG	equ	$ff90
GIMECSZ	equ	$10
GIMEIRQ	equ	$ff92
GIMEFRQ	equ	$ff93

PALBASE	equ	$ffb0
PALETSZ	equ	$10

SAMR1CL	equ	$ffd8
SAMR1ST	equ	$ffd9

AUBUFSZ	equ	$00b8

VIDBASE	equ	$1c00
VIDSIZE	equ	$3000

VMODEMX	equ	6
PALETMX	equ	5

* Frame step value should be 2x actual frame step for 30fps source video
* FRAMSTP	equ	1

* 325.1 for 11025 Hz
TIMEVAL	equ	325
TVH	equ	TIMEVAL/256
TVL	equ	TIMEVAL%256

	org	LOAD

* Loader can poke start sector for video data here
VIDSTRT	fcb	$00,$00,$00,$00
VIDMODE	fcb	$00
PALETTE	fcb	$00

EXEC	equ	*

* Set direct page register
	lda	#$ff
	tfr	a,dp
	setdp	$ff

* Disable IRQ and FIRQ
	orcc	#$50

* Initialize PIAs
* - Disable PIA[01] interrupt generation
* - Select internal DAC for sound output
* - Enable sound output
	lda	#$34
	sta	PIA0C0
	lda	#$34
	sta	PIA0C1
	lda	#$30
	sta	PIA1C0
	lda	#$fc
	sta	PIA1D0
	lda	#$34
	sta	PIA1C0
	lda	#$38
	sta	PIA1C1
* Bleed-off PIA[01] interrupts
	lda	PIA0D0
	lda	PIA0D1
	lda	PIA1D0
	lda	PIA1D1

* High-speed poke...definitely...
	sta	SAMR1ST

* Setup palette...
	ldx	#PALINIT
	lda	PALETTE
	cmpa	#PALETMX	; avoid unknown palettes
	lbgt	EXIT
	lsla			; multiply PALETTE by size of palette regs
	lsla
	lsla
	lsla
	leax	a,x		; load offset to desired palette
	lda	#PALETSZ
	pshs	a
	ldy	#PALBASE
PINLOOP	lda	,x+
	sta	,y+
	dec	,s
	bne	PINLOOP
	leas	1,s

* ...clear screen...
	ldx	#VIDBASE
	clra
CLRSCN	sta	,x+
	cmpx	#(VIDBASE+VIDSIZE)
	bne	CLRSCN

* ...and setup GIME registers
	ldx	#GIMEINI
	lda	VIDMODE
	cmpa	#VMODEMX	; avoid unsupported video modes
	lbgt	EXIT
	lsla			; multiply VIDMODE by size of GIME config
	lsla
	lsla
	lsla
	leax	a,x		; load offset to desired GIME config
	lda	#GIMECSZ
	pshs	a
	ldy	#GIMECFG
GINLOOP	lda	,x+
	sta	,y+
	dec	,s
	bne	GINLOOP
	leas	1,s

* Clear audio buffer
	ldx	#AUBUFST
	clra
CLRAUBF	sta	,x+
	cmpx	#(AUBUFST+2*AUBUFSZ)
	bne	CLRAUBF

* Init audio buffer switching and video frame step
* (Frame step probably can be eliminated...?)
*	lda	#FRAMSTP
*	sta	STEPCNT
	clr	FRAMCNT
	ldd	#AUBUFST
	std	AUDWPTR
	addd	#AUBUFSZ
	std	AUDWSTP
	std	AUDRPTR
	tfr	d,y

* Init Vsync interrupt generation
	init_video_timer

* Init (audio) timer interrupt generation
	init_audio_timer

* Init storage access
	init_storage

* Enable Vsync and (audio) timer interrupts
	andcc	#$af

* start new video data segment
VIDLOOP	data_read
* check for jump
	cmpa	#$ff
	beq	VIDEOF
* save first byte of control word
	pshs	a
* mask/shift d to retrieve offset
	anda	#$f8
	lsra
	lsra
	lslb
	bcc	VIDJUMP
	ora	#$01
* load x w/ start of video buffer
VIDJUMP	ldx	#VIDBASE
* adjust x to correct offset
	leax	d,x
* convert a to word count
	lda	#$07
	anda	,s
	inca
	sta	,s
VIDSTOR	data_read
	std	,x++
	dec	,s
	bne	VIDSTOR
* jump to outer loop
	leas	1,s
	bra	VIDLOOP

* check for end of video
VIDEOF	cmpb	#$ff
	beq	EXIT
* else, fall-through to audio processing

* Audio data movement goes here
	ldx	AUDWPTR
AUDLOOP	data_read
* Store data in audio buffer
	std	,x++
	cmpx	AUDWSTP
	blt	AUDLOOP

* Synchronize!  Wait for data pump semaphore...
SYNC	lda	FRAMCNT
	beq	SYNC
* Save pointer to current frames for playback
	ldd	AUDRPTR
* Disable Vsync and (audio) timer interrupts
	orcc	#$50
* Decrement data pump semaphore
	dec	FRAMCNT
* Switch to next audio frame for playback
	ldy	AUDWPTR		; y is used as actual ptr for playback
	sty	AUDRPTR		; old write ptr becomes new read ptr
* Enable Vsync and (audio) timer interrupts ASAP
	andcc	#$af
* Switch to next audio frame for loading samples
	std	AUDWPTR		; old read ptr becomes new write ptr
	addd	#AUBUFSZ
	std	AUDWSTP		; store new write buffer end ptr
	bra	VIDLOOP

* Execute reset vector
* (needs work -- makes it back to RSDOS prompt, but still in high-speed mode)
EXIT	orcc	#$50
	clr	RSTFLG
	jmp	[$fffe]

* Instantiate body of storage driver
	storage_handler

* Handle Vsync interrupt
	video_timer_handler

* Handle (audio) timer interrupt
	audio_timer_handler

* Init for video mode, set video buffer to VIDBASE
* (Assumes default MMU setup...)
GIMEINI	equ	*
MODE0	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$12,$00,$00,$0f,$e3,$80,$00
MODE1	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$12,$00,$00,$0f,$e3,$80,$00
MODE2	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$19,$00,$00,$0f,$e3,$80,$00
MODE3	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$00,$00,$00,$0f,$e3,$80,$00
MODE4	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$08,$00,$00,$0f,$e3,$80,$00
MODE5	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$10,$00,$00,$0f,$e3,$80,$00
MODE6	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$09,$00,$00,$0f,$e3,$80,$00

* Init for palette regs
PALINIT	equ	*
RGB16	fcb	$00,$09,$12,$1b,$24,$2d,$36,$3f
	fcb	$07,$0e,$15,$1c,$23,$2a,$31,$38
CMP16	fcb	$00,$2c,$12,$2e,$27,$29,$24,$30
	fcb	$10,$0a,$01,$0f,$06,$1a,$04,$20
CMP256	fcb	$00,$10,$20,$30,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
RGB2	fcb	$00,$3f,$00,$00,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
CMP2	fcb	$00,$30,$00,$00,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
RGB4S0	fcb	$12,$36,$09,$24,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
CMP4S0	fcb	$12,$24,$2e,$27,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
RGB4S1	fcb	$3f,$1a,$2b,$26,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
CMP4S1	fcb	$30,$1f,$3a,$26,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00

* STEPCNT	rmb	1
FRAMCNT	rmb	1

AUDRPTR	rmb	2
AUDWPTR	rmb	2
AUDWSTP	rmb	2

* Allocate audio buffers
AUBUFST	rmb	2*AUBUFSZ

* Allocate any variables used by storage driver
	storage_variables

	end	EXEC
