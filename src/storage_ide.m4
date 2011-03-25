dnl
dnl Initialize storage driver
dnl
define(`init_storage',`
* Init IDE drive
	ldu	#$ff50
	clrb
* Clear pending data transfers
ideclr	lda	7,u
	anda	#8
	beq	idecal
	lda	,u
	lda	8,u
	bra	ideclr
* Issue recalibrate command
idecal	clr	1,u
	clr	2,u
	clr	3,u
	clr	4,u
	clr	5,u
	lda	#160
	sta	6,u
	lda	#16	
	sta	7,u
iderec1	lda	7,u
	anda	#128
	bne	iderec1
* Assume it worked -- what else would we do?

* Now issue first read command
	clr	1,u
* For now, presume 256-sector padding
	clr	2,u
	ldx	#VIDSTRT
	lda	3,x
	sta	3,u
	lda	2,x
	sta	4,u
	lda	1,x
	sta	5,u
	lda	,x
	ora	#224
	sta	6,u
	lda	#32	
	sta	7,u
ideread	lda	7,u
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
	bitb	7,u
	bne	datrdcont(datrditer)
	lbsr	DATREQ
datrdcont(datrditer)	lda	,u
	ldb	8,u
')dnl
dnl
dnl Single instance body of storage driver
dnl    -- Called to fill request new reads
dnl    -- Also checks keyboard input
dnl
define(`storage_handler',`
* Increment LBA and request next sector
DATREQ	clr	2,u
	clr	3,u
	inc	4,u
	bne	DATCMD
	inc	5,u
	bne	DATCMD
	ldb	6,u
	incb
	andb	#15
	orb	#224
	stb	6,u
DATCMD	ldb	#32
	stb	7,u
DATWAIT ldb	7,u
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
