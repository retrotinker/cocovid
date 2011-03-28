.PHONY: all clean

CFLAGS=-Wall

TARGETS=ppm2raw raw2ppm raw2frm frm2raw frmprune \
	colormap.ppm START.frm END.frm BLACK.frm

OTHERS=gencolors genmap

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

ppm2raw: ppm2raw.c colors.h
	$(CC) $(CFLAGS) -o ppm2raw ppm2raw.c

raw2ppm: raw2ppm.c palette.h
	$(CC) $(CFLAGS) -o raw2ppm raw2ppm.c

raw2frm: raw2frm.c
	$(CC) $(CFLAGS) -o raw2frm raw2frm.c

frmprune: frmprune.c distance.h
	$(CC) $(CFLAGS) -o frmprune frmprune.c

frm2raw: frm2raw.c
	$(CC) $(CFLAGS) -o frm2raw frm2raw.c

START.frm: START.dat
	xxd -r $< $@

END.frm: END.dat
	xxd -r $< $@

WHITE.raw: WHITE.dat
	xxd -r $< $@

BLACK.raw: BLACK.dat
	xxd -r $< $@

BLACK.frm: BLACK.raw WHITE.raw
	./raw2frm WHITE.raw BLACK.raw $@
