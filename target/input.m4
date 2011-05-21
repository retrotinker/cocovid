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

define(`check_keyboard',`
* Check for user stop request
KEYCHK	pshs	a
	jsr	[$A000]
	beq	KEYCHK1
	cmpa	#03
	lbeq	EXIT
* Check for pause
	cmpa	#32
	beq	KEYCHK2
KEYCHK1	puls	a
	bra	KEYCHK4
* Disable IRQ and FIRQ
KEYCHK2	orcc	#80
* Pause until user hits space again
KEYCHK3	jsr	[$A000]
	beq	KEYCHK3
	cmpa	#03
	beq	EXIT
	cmpa	#32
	bne	KEYCHK3
	puls	a
* Enable IRQ and FIRQ
	andcc	#175
KEYCHK4	equ	*
')dnl
