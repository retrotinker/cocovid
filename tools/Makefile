.PHONY: all clean

CFLAGS=-Wall

TARGETS=ppm2frm0 frm2raw0 raw2ppm0 \
	ppm2frm1 frm2raw1 raw2ppm1 \
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

raw2ppm0: raw2ppm.c palette.h
	$(CC) $(CFLAGS) -DMODE=0 -o $@ raw2ppm.c

frm2raw0: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=0 -o $@ frm2raw.c

ppm2frm0: ppm2frm.c colors.h distance.h
	$(CC) $(CFLAGS) -DMODE=0 -o $@ ppm2frm.c

raw2ppm1: raw2ppm.c palette.h
	$(CC) $(CFLAGS) -DMODE=1 -o $@ raw2ppm.c

frm2raw1: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=1 -o $@ frm2raw.c

ppm2frm1: ppm2frm.c colors.h distance.h
	$(CC) $(CFLAGS) -DMODE=1 -o $@ ppm2frm.c

START.frm: START.dat
	xxd -r $< $@

END.frm: END.dat
	xxd -r $< $@

# This is used in all modes
# -- OK so long as the video buffer is always max size
BLACK.raw: BLACK.dat
	xxd -r $< $@
