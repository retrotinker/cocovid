all: cocovid.bin cocovid.s19 cocovid-raw.bin cocovid-raw.s19 \
	cocovid-poc.bin cocovid-poc.s19

cocovid.bin: cocovid.asm
	mamou -mb -tb -l -ococovid.bin cocovid.asm

cocovid.s19: cocovid.asm
	mamou -mb -ts -l -ococovid.s19 cocovid.asm

cocovid-raw.bin: cocovid-raw.asm
	mamou -mb -tb -l -ococovid-raw.bin cocovid-raw.asm

cocovid-raw.s19: cocovid-raw.asm
	mamou -mb -ts -l -ococovid-raw.s19 cocovid-raw.asm

cocovid-poc.bin: cocovid-poc.asm
	mamou -mb -tb -l -ococovid-poc.bin cocovid-poc.asm

cocovid-poc.s19: cocovid-poc.asm
	mamou -mb -ts -l -ococovid-poc.s19 cocovid-poc.asm
