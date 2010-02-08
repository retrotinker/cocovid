dnl
dnl Macros for checking/handling audio timer expiration
dnl
define(`ckauditer', 0)dnl
define(`ckaudentlbl', `CKAUD$1')dnl
define(`ckaudextlbl', `CKAUDX$1')dnl
define(`ckaudentry', `define(`ckauditer', incr(ckauditer))dnl
ckaudentlbl(ckauditer)')dnl
define(`ckaudexit', `ckaudextlbl(ckauditer)')dnl
define(`check_audio_timer',`
ckaudentry(ckauditer)	tst	$ff93
	beq	ckaudexit(ckauditer)
* Load and play sample
	lda	,y+
	sta	$ff20
	cmpy	>AUDRSTP
	blt	ckaudexit(ckauditer)
	leay	-1,y
ckaudexit(ckauditer)	equ	*')dnl
dnl
define(`init_video_timer',`
	lda	$ff92
	ora	#08
	sta	$ff92
	lda	$ff90
	ora	#32
	sta	$ff90
')dnl
dnl
dnl Macros for checking/handling video timer expiration
dnl
define(`ckviditer', 0)dnl
define(`ckvidentlbl', `CKVID$1')dnl
define(`ckvidextlbl', `CKVIDX$1')dnl
define(`ckvidentry', `define(`ckviditer', incr(ckviditer))dnl
ckvidentlbl(ckviditer)')dnl
define(`ckvidexit', `ckvidextlbl(ckviditer)')dnl
define(`check_video_timer',`
ckvidentry(ckviditer)	tst	$ff92
	beq	ckvidexit(ckviditer)
	lbsr	VIDTMR
ckvidexit(ckviditer)	equ	*')dnl
dnl
define(`video_timer_handler',`
VIDTMR	pshs	a
* Account for frame timing
	DEC	>STEPCNT
	BNE	VIDTMR2
* Unblock data pump -- limit FRAMCNT to prevent catch-up silliness
	LDA	>FRAMCNT
	INCA
	ANDA	#07
	STA	>FRAMCNT
* Reset frame skip count
	LDA	#FRAMSTP
	STA	>STEPCNT
VIDTMR2 puls	a
	RTS
')dnl
define(`ckviditer', 0)dnl
define(`check_timers', `check_audio_timer()check_video_timer()')dnl
