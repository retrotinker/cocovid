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
VHDRECH	equ	$ff80
VHDRECM	equ	$ff81
VHDRECL	equ	$ff82
VHDCMDS	equ	$ff83
VHDBUFH	equ	$ff84
VHDBUFL	equ	$ff85

	ldx	#VIDSTRT
	lda	1,x
	sta	VHDLRNH
	sta	VHDRECH
	lda	2,x
	sta	VHDLRNM
	sta	VHDRECM
	lda	3,x
	sta	VHDLRNL
	sta	VHDRECL
	ldd	#VHDBUF
	sta	VHDBUFH
	stb	VHDBUFL
* Read first sector
	clr	VHDCMDS
* Check status
	lda	VHDCMDS
* Exit on error
	lbne	EXIT
* Point at buffer base
	ldu	#VHDBUF
')dnl
dnl
dnl Read next 2 bytes from storage into D accumulator
dnl
define(`datrditer', 0)dnl
define(`datrdentlbl', `DATRD$1')dnl
define(`datrdextlbl', `DATRDX$1')dnl
define(`datrdentry', `define(`datrditer', incr(datrditer))dnl
datrdentlbl(datrditer)')dnl
define(`datrdexit', `datrdextlbl(datrditer)')dnl
define(`data_read',`
datrdentry(datrditer)	ldd	,u++
	cmpu	#VHDBEND
	bne	datrdexit(datrditer)
	bsr	DATREQ
datrdexit(datrditer)	equ	*')dnl
dnl
dnl Single instance body of storage driver
dnl    -- Called to fill request new reads
dnl    -- Also checks keyboard input
dnl
define(`storage_handler',`
* Increment LBA and request next sector
DATREQ	pshs	b
	inc	VHDLRNL
	ldb	VHDLRNL
	stb	VHDRECL
	bne	DATCMD
	inc	VHDLRNM
	ldb	VHDLRNM
	stb	VHDRECM
	bne	DATCMD
	inc	VHDLRNH
	ldb	VHDLRNH
	stb	VHDRECH
DATCMD	clr	VHDCMDS
	ldb	VHDCMDS
* Abort on error...ick...
	lbne	EXIT
	puls	b
	ldu	#VHDBUF
* Is this really the best time to check
* the keyboard??
	check_keyboard
	rts
')dnl
dnl
dnl Single instance static allocation for storage driver usage
dnl
define(`storage_variables',`
VHDLRNH	rmb	1
VHDLRNM	rmb	1
VHDLRNL	rmb	1
VHDBUF	rmb	256
VHDBEND	equ	*
')dnl
dnl
dnl Clean-up any pending data transfers
dnl
define(`storage_cleanup',`
* No storage cleanup necessary for VHD driver
')dnl
