dnl
dnl Initialize storage driver
dnl
`errprint(`init_storage undefined!')m4exit(1)'
dnl
dnl Read next byte from storage into A accumulator
dnl    -- Uses B accumulator for tracking next read location
dnl
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
dnl
dnl Single instance body of storage driver
dnl    -- Called to fill request new reads
dnl    -- Also checks keyboard input
dnl
`errprint(`storage_handler undefined!')m4exit(1)'
dnl
dnl Single instance static allocation for storage driver usage
dnl
`errprint(`storage_variables undefined!')m4exit(1)'
