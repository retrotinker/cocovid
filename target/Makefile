.PHONY: all clean

TARGETS := cocovhd.bin cocovhd.s19 cocoide.bin cocoide.s19
GENERATED := cocovhd.asm cocoide.asm

BASE_MACROS := timers.m4 input.m4 storage.m4
VHD_MACROS := storage_vhd.m4
IDE_MACROS := storage_ide.m4

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(GENERATED)

cocovhd.asm: cocovid.m4 $(BASE_MACROS) $(VHD_MACROS)
	if ! m4 -D storage=vhd $< > $@; then rm -f $@; exit 1; fi

cocoide.asm: cocovid.m4 $(BASE_MACROS) $(IDE_MACROS)
	if ! m4 -D storage=ide $< > $@; then rm -f $@; exit 1; fi

cocovhd.bin: cocovhd.asm
	mamou -mb -tb -l -ococovhd.bin cocovhd.asm

cocovhd.s19: cocovhd.asm
	mamou -mb -ts -l -ococovhd.s19 cocovhd.asm

cocoide.bin: cocoide.asm
	mamou -mb -tb -l -ococoide.bin cocoide.asm

cocoide.s19: cocoide.asm
	mamou -mb -ts -l -ococoide.s19 cocoide.asm
