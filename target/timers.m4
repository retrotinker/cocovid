dnl
dnl Macro for initializing audio timer
dnl
define(`init_audio_timer',`
*	lda	#126
*	sta	FIRQJMP
	ldd	#AUDISR
	std	FIRQADR
')dnl
dnl
dnl Macro for handling audio timer expiration
dnl
define(`audio_timer_handler',`
AUDISR	tst	GIMEFRQ
	pshs	a
* Load and play sample
	lda	,y+
	sta	PIA1D0
	puls	a
	rti
')dnl
dnl
dnl Macro for initializing video timer
dnl
define(`init_video_timer',`
*	lda	#126
*	sta	IRQJMP
	ldd	#VIDISR
	std	IRQADR
')dnl
dnl
dnl Macro for handling video timer expiration
dnl
define(`video_timer_handler',`
VIDISR	tst	GIMEIRQ
* Account for frame timing
*	dec	STEPCNT
*	bne	VIDISR1
* Increment data pump semaphore
	inc	FRAMCNT
* Reset frame skip count
*	lda	#FRAMSTP
*	sta	STEPCNT
VIDISR1 rti
')dnl
