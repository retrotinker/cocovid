.PHONY: all clean

TARGETS=cocovid.bin cocovid.s19 cocoaud.bin cocoaud.s19

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

cocovid.bin: cocovid.asm
	mamou -mb -tb -l -ococovid.bin cocovid.asm

cocovid.s19: cocovid.asm
	mamou -mb -ts -l -ococovid.s19 cocovid.asm

cocoaud.bin: cocoaud.asm
	mamou -mb -tb -l -ococoaud.bin cocoaud.asm

cocoaud.s19: cocoaud.asm
	mamou -mb -ts -l -ococoaud.s19 cocoaud.asm
