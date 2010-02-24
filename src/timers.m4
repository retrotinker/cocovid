dnl
dnl Macro for initializing audio timer
dnl
define(`init_audio_timer',`
	ldd	#TIMEVAL
	std	$ff94
	lda	$ff91
	ora	#32
	sta	$ff91
	lda	#32
	sta	$ff93
	lda	$ff90
	ora	#16
	sta	$ff90
	lda	#126
	sta	>FIRQJMP
	ldd	#AUDISR
	std	>FIRQADR
')dnl
dnl
dnl Macro for handling audio timer expiration
dnl
define(`audio_timer_handler',`
AUDISR	pshs	a
	tst	$ff93
	beq	AUDISR1
* Load and play sample
	lda	,y+
	sta	$ff20
	cmpy	>AUDRSTP
	blt	AUDISR1
	leay	-1,y
AUDISR1	puls	a
	rti
')dnl
dnl
dnl Macro for initializing video timer
dnl
define(`init_video_timer',`
	lda	#126
	sta	>IRQJMP
	ldd	#VIDISR
	std	>IRQADR
')dnl
dnl
dnl Macro for handling video timer expiration
dnl
define(`video_timer_handler',`
VIDISR	tst	$ff02
* Account for frame timing
	DEC	>STEPCNT
	BNE	VIDISR1
* Unblock data pump -- limit FRAMCNT to prevent catch-up silliness
	LDA	>FRAMCNT
	INCA
	ANDA	#07
	STA	>FRAMCNT
* Reset frame skip count
	LDA	#FRAMSTP
	STA	>STEPCNT
VIDISR1 rti
')dnl
