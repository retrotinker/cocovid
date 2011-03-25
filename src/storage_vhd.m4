dnl
dnl Initialize storage driver
dnl
define(`init_storage',`
	ldu	#$ff80
	ldx	#VIDSTRT
	lda	3,x
	sta	>VHDLRNH
	sta	,u
	lda	2,x
	sta	>VHDLRNM
	sta	1,u
	lda	1,x
	sta	>VHDLRNH
	sta	2,u
	ldd	#VHDBUF
	sta	4,u
	stb	5,u
* Read first sector
	clr	3,u
* Check status
	lda	3,u
* Exit on error
	lbne	EXIT
* Point at buffer base
	ldu	#VHDBUF
	clrb
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
	lbsr	DATREQ
datrdexit(datrditer)	equ	*')dnl
dnl
dnl Single instance body of storage driver
dnl    -- Called to fill request new reads
dnl    -- Also checks keyboard input
dnl
define(`storage_handler',`
* Increment LBA and request next sector
DATREQ	ldu	#$ff80
	inc	>VHDLRNL
	pshs	b
	ldb	>VHDLRNL
	stb	2,u
	bne	DATCMD
	inc	>VHDLRNM
	ldb	>VHDLRNM
	stb	1,u
	bne	DATCMD
	inc	>VHDLRNH
	ldb	>VHDLRNH
	stb	,u
DATCMD	clr	3,u
	ldb	3,u
	puls	b
* Abort on error...ick...
	bne	EXIT
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
