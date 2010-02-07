define(`init_storage',`
	ldu	#$ff80
	clr	,u
	clr	1,u
	clr	2,u
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
* Clear stored record address
	clr	>VHDLRNH
	clr	>VHDLRNM
	clr	>VHDLRNL
')dnl
define(`datrditer', 0)dnl
define(`datrdentlbl', `DATRD$1')dnl
define(`datrdextlbl', `DATRDX$1')dnl
define(`datrdentry', `define(`datrditer', incr(datrditer))dnl
datrdentlbl(datrditer)')dnl
define(`datrdexit', `datrdextlbl(datrditer)')dnl
define(`data_read',`
datrdentry(datrditer)	lda	b,u
	incb
	cmpb	#128
	bne	datrdexit(datrditer)
	lbsr	DATREQ
datrdexit(datrditer)	equ	*')dnl
define(`storage_handler',`
* Increment LBA and request next sector
DATREQ	cmpu	#VHDBFHI
	beq	DATREQ1
	ldu	#VHDBFHI
	clrb
	rts
DATREQ1
	ldu	#$ff80
	inc	>VHDLRNL
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
DATCMD
	clr	3,u
	ldb	3,u
* Abort on error...ick...
	bne	EXIT
	ldu	#VHDBUF
	clrb
	check_keyboard
	rts
')dnl
define(`storage_variables',`
VHDLRNH	RMB	1
VHDLRNM	RMB	1
VHDLRNL	RMB	1
VHDBUF	RMB	128
VHDBFHI	RMB	128
')dnl
