.PHONY: all clean

TARGETS := cocovid.bin cocovid.s19 cocovhd.bin cocovhd.s19 \
	cocoide.bin cocoide.s19 \
	cocoaud.bin cocoaud.s19 cocolaw.bin cocolaw.s19
GENERATED := cocovhd.asm

BASE_MACROS := timers.m4 input.m4 storage.m4
VHD_MACROS := storage_vhd.m4
IDE_MACROS := storage_ide.m4

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(GENERATED)

cocovid.bin: cocovid.asm
	mamou -mb -tb -l -ococovid.bin cocovid.asm

cocovid.s19: cocovid.asm
	mamou -mb -ts -l -ococovid.s19 cocovid.asm

cocovhd.asm: cocovhd.m4 $(BASE_MACROS) $(VHD_MACROS)
	if ! m4 -D storage=vhd $< > $@; then rm -f $@; exit 1; fi

cocoide.asm: cocovhd.m4 $(BASE_MACROS) $(IDE_MACROS)
	if ! m4 -D storage=ide $< > $@; then rm -f $@; exit 1; fi

cocovhd.bin: cocovhd.asm
	mamou -mb -tb -l -ococovhd.bin cocovhd.asm

cocovhd.s19: cocovhd.asm
	mamou -mb -ts -l -ococovhd.s19 cocovhd.asm

cocoide.bin: cocoide.asm
	mamou -mb -tb -l -ococoide.bin cocoide.asm

cocoide.s19: cocoide.asm
	mamou -mb -ts -l -ococoide.s19 cocoide.asm

cocoaud.bin: cocoaud.asm
	mamou -mb -tb -l -ococoaud.bin cocoaud.asm

cocoaud.s19: cocoaud.asm
	mamou -mb -ts -l -ococoaud.s19 cocoaud.asm

cocolaw.bin: cocolaw.asm
	mamou -mb -tb -l -ococolaw.bin cocolaw.asm

cocolaw.s19: cocolaw.asm
	mamou -mb -ts -l -ococolaw.s19 cocolaw.asm
