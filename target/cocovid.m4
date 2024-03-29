*
* Copyright (c) 2009-2011, John W. Linville <linville@tuxdriver.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*

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
LOAD	equ	$4800		Actual load address for binary

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

AUBUFSZ	equ	$00b8	; this needs to be an even number...

VIDBASE	equ	$5000
VIDSIZE	equ	$3000

VMODEMX	equ	$07
PALETMX	equ	$0a

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

* Save machine state for clean exit
* - save registers
* - save stack pointer
	pshs	cc,a,b,dp,x,y,u
	sts	>SAVESTK
	lds	#VIDBASE
* - save PIA0/1 state
	ldx	#$ff00
	ldy	#SAVPIA0
	lbsr	SAVPIA
	ldx	#$ff20
	ldy	#SAVPIA1
	lbsr	SAVPIA
* - save IRQ/FIRQ handlers
	ldd	IRQADR
	std	>SAVIRQH
	ldd	FIRQADR
	std	>SAVFRQH
* - save palette values
	lda	#PALETSZ
	pshs	a
	ldx	#PALBASE
	ldy	#SAVEPAL
PSVLOOP	lda	,x+
	sta	,y+
	dec	,s
	bne	PSVLOOP
	leas	1,s

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
	lda	#$3c
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
	ldb	PALETTE
	cmpb	#PALETMX	; avoid unknown palettes
	lbgt	EXIT
	clra
	lslb			; multiply PALETTE by size of palette regs
	rola
	lslb
	rola
	lslb
	rola
	lslb
	rola
	leax	d,x		; load offset to desired palette
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
	ldb	VIDMODE
	cmpb	#VMODEMX	; avoid unsupported video modes
	lbgt	EXIT
	lslb			; multiply VIDMODE by size of GIME config
	rola
	lslb
	rola
	lslb
	rola
	lslb
	rola
	leax	b,x		; load offset to desired GIME config
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
* save second byte of control word
	pshs	b
* mask/shift d to retrieve offset
	andb	#$f8
	lsra
	rorb
	lsra
	rorb
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
	lbeq	EXIT
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

* Instantiate body of storage driver
	storage_handler

* Handle Vsync interrupt
	video_timer_handler

* Handle (audio) timer interrupt
	audio_timer_handler

* Routine to save PIA state (X is PIA base, Y is save buffer)
SAVPIA	lda	1,x
	sta	,y
	anda	#$fb
	sta	1,x	; select direction register
	ldb	,x
	stb	1,y
	ora	#$04
	sta	1,x	; select output register
	ldb	,x
	stb	2,y
* Second half, just like the first...
	lda	3,x
	sta	3,y
	anda	#$fb
	sta	3,x	; select direction register
	ldb	2,x
	stb	4,y
	ora	#$04
	sta	3,x	; select output register
	ldb	2,x
	stb	5,y
	rts

* Routine to restore PIA state (X is PIA base, Y is save buffer)
RSTPIA	lda	1,x
	anda	#$fb
	sta	1,x	; select direction register
	ldb	1,y
	stb	,x
	ora	#$04
	sta	1,x	; select output register
	ldb	2,y
	stb	,x
	lda	,y
	sta	1,x
* Second half, just like the first...
	lda	3,x
	anda	#$fb
	sta	3,x	; select direction register
	ldb	4,y
	stb	2,x
	ora	#$04
	sta	3,x	; select output register
	ldb	5,y
	stb	2,x
	lda	3,y
	sta	3,x
	rts

* Restore machine state for clean exit
EXIT	orcc	#$50
* - hit low-speed poke
	sta	SAMR1CL
* - load default GIME config
	lda	#GIMECSZ
	pshs	a
	ldx	#GIMEDEF
	ldy	#GIMECFG
GDEFLOP	lda	,x+
	sta	,y+
	dec	,s
	bne	GDEFLOP
	leas	1,s
* - restore palette values
	lda	#PALETSZ
	pshs	a
	ldx	#SAVEPAL
	ldy	#PALBASE
PRSTLOP	lda	,x+
	sta	,y+
	dec	,s
	bne	PRSTLOP
	leas	1,s
* - restore IRQ/FIRQ handlers
	ldd	SAVIRQH
	std	IRQADR
	ldd	SAVFRQH
	std	FIRQADR
* - restore PIA0/1 state
	ldx	#$ff00
	ldy	#SAVPIA0
	bsr	RSTPIA
	ldx	#$ff20
	ldy	#SAVPIA1
	bsr	RSTPIA
* Clean-up pending storage transfers...
	storage_cleanup
* - restore stack pointer
* - restore registers
	lds	SAVESTK
	puls	cc,a,b,dp,x,y,u
	rts

* Init for video mode, set video buffer to VIDBASE
* (Assumes default MMU setup...)
GIMEINI	equ	*
MODE0	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$12,$00,$00,$0f,$ea,$00,$00
MODE1	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$12,$00,$00,$0f,$ea,$00,$00
MODE2	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$19,$00,$00,$0f,$ea,$00,$00
MODE3	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$00,$00,$00,$0f,$ea,$00,$00
MODE4	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$08,$00,$00,$0f,$ea,$00,$00
* Modes 5-7 are identical on the CoCo3, replicated for simplicity...
MODE5	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$09,$00,$00,$0f,$ea,$00,$00
MODE6	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$82,$09,$00,$00,$0f,$ea,$00,$00
MODE7	fcb	$7c,$20,$08,$20,TVH,TVL,$00,$00
	fcb	$80,$09,$00,$00,$0f,$ea,$00,$00

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
CMP4S0	fcb	$12,$24,$0b,$07,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
RGB4S1	fcb	$3f,$1b,$2d,$26,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
CMP4S1	fcb	$3f,$1f,$09,$26,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
RGB4A	fcb	$00,$0a,$22,$3f,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00
CMP4A	fcb	$00,$0d,$26,$30,$00,$00,$00,$00
	fcb	$00,$00,$00,$00,$00,$00,$00,$00

GIMEDEF	fcb	$c4,$00,$00,$00,$ff,$ff,$00,$00
	fcb	$00,$00,$00,$00,$0f,$e0,$00,$00

SAVESTK	rmb	2
SAVPIA0	rmb	6
SAVPIA1	rmb	6
SAVIRQH	rmb	2
SAVFRQH	rmb	2
SAVEPAL	rmb	16

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
