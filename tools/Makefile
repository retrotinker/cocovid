.PHONY: all clean

CFLAGS=-Wall

TARGETS=ppm2frm0 frm2raw0 raw2ppm0 \
	ppm2frm1 frm2raw1 raw2ppm1 \
	ppm2frm2 frm2raw2 raw2ppm2 \
	ppm2frm3 frm2raw3 raw2ppm3 \
	ppm2frm4 frm2raw4 raw2ppm4 \
	START.frm END.frm \
	colormap16.ppm colormap256.ppm

OTHERS=gencolors16 genmap16 colors16.h distance16.h \
	gencolors256 genmap256 colors256.h distance256.h

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OTHERS)

START.frm: START.dat
	xxd -r $< $@

END.frm: END.dat
	xxd -r $< $@

# This is used in all modes
# -- OK so long as the video buffer is always max size
BLACK.raw: BLACK.dat
	xxd -r $< $@

#
# I hate all this generation of multiple binaries from a single
# source.  But, by the time I got to this point the utilities were
# already written/working with a lot of staticly sized declarations
# and the like.  Still, the binaries are small.  So in the interest
# of having working code, this is what I'm doing...
#
gencolors16: gencolors.c palette16.h
	$(CC) $(CFLAGS) -o $@ -DCOLORS=16 gencolors.c -lm

gencolors256: gencolors.c palette256.h
	$(CC) $(CFLAGS) -o $@ -DCOLORS=256 gencolors.c -lm

colors16.h: gencolors16
	./gencolors16 > $@

colors256.h: gencolors256
	./gencolors256 > $@

distance16.h: gencolors16
	./gencolors16 -d > $@

distance256.h: gencolors256
	./gencolors256 -d > $@

colormap16.ppm: genmap16
	./genmap16 > $@

colormap256.ppm: genmap256
	./genmap256 > $@

genmap16: genmap.c palette16.h
	$(CC) $(CFLAGS) -DCOLORS=16 -o $@ genmap.c

genmap256: genmap.c palette256.h
	$(CC) $(CFLAGS) -DCOLORS=256 -o $@ genmap.c

raw2ppm0: raw2ppm.c palette16.h
	$(CC) $(CFLAGS) -DMODE=0 -o $@ raw2ppm.c

frm2raw0: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=0 -o $@ frm2raw.c

ppm2frm0: ppm2frm.c colors16.h distance16.h
	$(CC) $(CFLAGS) -DMODE=0 -o $@ ppm2frm.c

raw2ppm1: raw2ppm.c palette16.h
	$(CC) $(CFLAGS) -DMODE=1 -o $@ raw2ppm.c

frm2raw1: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=1 -o $@ frm2raw.c

ppm2frm1: ppm2frm.c colors16.h distance16.h
	$(CC) $(CFLAGS) -DMODE=1 -o $@ ppm2frm.c

raw2ppm2: raw2ppm.c palette256.h
	$(CC) $(CFLAGS) -DMODE=2 -o $@ raw2ppm.c

frm2raw2: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=2 -o $@ frm2raw.c

ppm2frm2: ppm2frm.c colors256.h distance256.h
	$(CC) $(CFLAGS) -DMODE=2 -o $@ ppm2frm.c

raw2ppm3: raw2ppm.c palette2.h
	$(CC) $(CFLAGS) -DMODE=3 -o $@ raw2ppm.c

frm2raw3: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=3 -o $@ frm2raw.c

ppm2frm3: ppm2frm.c distance2.h
	$(CC) $(CFLAGS) -DMODE=3 -o $@ ppm2frm.c

raw2ppm4: raw2ppm.c palette2.h
	$(CC) $(CFLAGS) -DMODE=4 -o $@ raw2ppm.c

frm2raw4: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=4 -o $@ frm2raw.c

ppm2frm4: ppm2frm.c distance2.h
	$(CC) $(CFLAGS) -DMODE=4 -o $@ ppm2frm.c
