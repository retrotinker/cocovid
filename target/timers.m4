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
