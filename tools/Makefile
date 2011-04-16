.PHONY: all clean

CFLAGS=-Wall

TARGETS=ppm2frm frm2raw raw2ppm \
	colormap.ppm START.frm END.frm

OTHERS=gencolors genmap colors.h distance.h

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OTHERS)

gencolors: gencolors.c palette.h
	$(CC) $(CFLAGS) -o gencolors gencolors.c -lm

colors.h: gencolors
	./gencolors > $@

distance.h: gencolors
	./gencolors -d > $@

colormap.ppm: genmap
	./genmap > $@

genmap: genmap.c palette.h
	$(CC) $(CFLAGS) -o genmap genmap.c

raw2ppm: raw2ppm.c palette.h
	$(CC) $(CFLAGS) -o raw2ppm raw2ppm.c

frm2raw: frm2raw.c
	$(CC) $(CFLAGS) -o frm2raw frm2raw.c

ppm2frm: ppm2frm.c colors.h distance.h
	$(CC) $(CFLAGS) -o ppm2frm ppm2frm.c

START.frm: START.dat
	xxd -r $< $@

END.frm: END.dat
	xxd -r $< $@

BLACK.raw: BLACK.dat
	xxd -r $< $@
