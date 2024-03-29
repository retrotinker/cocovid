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
dnl Initialize storage driver
dnl
define(`init_storage',`
IDEDATL	equ	$ff50
IDEERR	equ	$ff51
IDECNT	equ	$ff52
IDELSNL	equ	$ff53
IDELSNM	equ	$ff54
IDELSNH	equ	$ff55
IDEDRVH	equ	$ff56
IDESTAT	equ	$ff57
IDEDATH	equ	$ff58

* Init IDE drive
* Clear pending data transfers
ideclr	lda	IDESTAT
	anda	#8
	beq	idecal
	lda	IDEDATL
	lda	IDEDATH
	bra	ideclr
* Issue recalibrate command
idecal	clr	IDEERR
	clr	IDECNT
	clr	IDELSNL
	clr	IDELSNM
	clr	IDELSNH
	lda	#160
	sta	IDEDRVH
	lda	#16	
	sta	IDESTAT
iderec1	lda	IDESTAT
	anda	#128
	bne	iderec1
* Assume it worked -- what else would we do?

* Now issue first read command
	clr	IDEERR
* For now, presume 256-sector padding
	clr	IDECNT
	ldx	#VIDSTRT
	lda	3,x
	sta	IDELSNL
	lda	2,x
	sta	IDELSNM
	lda	1,x
	sta	IDELSNH
	lda	,x
	ora	#224
	sta	IDEDRVH
	lda	#32	
	sta	IDESTAT
ideread	lda	IDESTAT
	anda	#128
	bne	ideread
')dnl
dnl
dnl Read next 2 bytes from storage into D accumulator
dnl
define(`datrditer', 0)dnl
define(`datrdentlbl', `DATRD$1')dnl
define(`datrdcntlbl', `DATRDC$1')dnl
define(`datrdentry', `define(`datrditer', incr(datrditer))dnl
datrdentlbl(datrditer)')dnl
define(`datrdcont', `datrdcntlbl(datrditer)')dnl
define(`data_read',`
* Read next byte from IDE device
datrdentry(datrditer)	ldb	#08
	bitb	IDESTAT
	bne	datrdcont(datrditer)
	bsr	DATREQ
datrdcont(datrditer)	lda	IDEDATL
	ldb	IDEDATH
')dnl
dnl
dnl Single instance body of storage driver
dnl    -- Called to fill request new reads
dnl    -- Also checks keyboard input
dnl
define(`storage_handler',`
* Increment LBA and request next sector
DATREQ	clr	IDECNT
	clr	IDELSNL
	inc	IDELSNM
	bne	DATCMD
	inc	IDELSNH
	bne	DATCMD
	ldb	IDEDRVH
	incb
	andb	#15
	orb	#224
	stb	IDEDRVH
DATCMD	ldb	#32
	stb	IDESTAT
DATWAIT ldb	IDESTAT
	andb	#128
	bne	DATWAIT
* Is this really the best time to check
* the keyboard??
	check_keyboard
	rts
')dnl
dnl
dnl Single instance static allocation for storage driver usage
dnl
define(`storage_variables',`
* No static allocations for IDE driver
')dnl
dnl
dnl Clean-up any pending data transfers
dnl
define(`storage_cleanup',`
* Bleed-off pending IDE transfers...
idecln	lda	IDESTAT
	anda	#8
	beq	idedone
	lda	IDEDATL
	lda	IDEDATH
	bra	idecln
idedone
')dnl
