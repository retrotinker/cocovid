.PHONY: all clean

CFLAGS=-Wall

TARGETS=ppm2frm0 frm2raw0 raw2ppm0 \
	ppm2frm1 frm2raw1 raw2ppm1 \
	ppm2frm2 frm2raw2 raw2ppm2 \
	ppm2frm3 frm2raw3 raw2ppm3 \
	ppm2frm4 frm2raw4 raw2ppm4 \
	ppm2frm5 frm2raw5 raw2ppm5 \
	ppm2frm6 frm2raw6 raw2ppm6 \
	ppm2frm7 frm2raw7 raw2ppm7 \
	START.frm END.frm \
	colormap16.ppm colormap256.ppm \
	colormap4s0.ppm colormap4s1.ppm colormap4a.ppm

OTHERS=gencolors16 genmap16 colors16.h distance16.h \
	gencolors256 genmap256 colors256.h distance256.h \
	gencolors4s0 genmap4s0 colors4s0.h distance4s0.h \
	gencolors4s1 genmap4s1 colors4s1.h distance4s1.h \
	gencolors4a  genmap4a  colors4a.h  distance4a.h

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

gencolors4s0: gencolors.c palette4.h
	$(CC) $(CFLAGS) -o $@ -DCOLORS=4 -DCOLORSET=0 gencolors.c -lm

gencolors4s1: gencolors.c palette4.h
	$(CC) $(CFLAGS) -o $@ -DCOLORS=4 -DCOLORSET=1 gencolors.c -lm

gencolors4a: gencolors.c palette4.h
	$(CC) $(CFLAGS) -o $@ -DCOLORS=4 -DCOLORSET=2 gencolors.c -lm

colors16.h: gencolors16
	./gencolors16 > $@

colors256.h: gencolors256
	./gencolors256 > $@

colors4s0.h: gencolors4s0
	./gencolors4s0 > $@

colors4s1.h: gencolors4s1
	./gencolors4s1 > $@

colors4a.h: gencolors4a
	./gencolors4a > $@

distance16.h: gencolors16
	./gencolors16 -d > $@

distance256.h: gencolors256
	./gencolors256 -d > $@

distance4s0.h: gencolors4s0
	./gencolors4s0 -d > $@

distance4s1.h: gencolors4s1
	./gencolors4s1 -d > $@

distance4a.h: gencolors4a
	./gencolors4a -d > $@

colormap16.ppm: genmap16
	./genmap16 > $@

colormap256.ppm: genmap256
	./genmap256 > $@

colormap4s0.ppm: genmap4s0
	./genmap4s0 > $@

colormap4s1.ppm: genmap4s1
	./genmap4s1 > $@

colormap4a.ppm: genmap4a
	./genmap4a > $@

genmap16: genmap.c palette16.h
	$(CC) $(CFLAGS) -DCOLORS=16 -o $@ genmap.c

genmap256: genmap.c palette256.h
	$(CC) $(CFLAGS) -DCOLORS=256 -o $@ genmap.c

genmap4s0: genmap.c palette4.h
	$(CC) $(CFLAGS) -DCOLORS=4 -DCOLORSET=0 -o $@ genmap.c

genmap4s1: genmap.c palette4.h
	$(CC) $(CFLAGS) -DCOLORS=4 -DCOLORSET=1 -o $@ genmap.c

genmap4a: genmap.c palette4.h
	$(CC) $(CFLAGS) -DCOLORS=4 -DCOLORSET=2 -o $@ genmap.c

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

raw2ppm5: raw2ppm.c palette4.h
	$(CC) $(CFLAGS) -DMODE=5 -DCOLORSET=0 -o $@ raw2ppm.c

frm2raw5: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=5 -o $@ frm2raw.c

ppm2frm5: ppm2frm.c colors4s0.h distance4s0.h
	$(CC) $(CFLAGS) -DMODE=5 -o $@ ppm2frm.c

raw2ppm6: raw2ppm.c palette4.h
	$(CC) $(CFLAGS) -DMODE=6 -DCOLORSET=1 -o $@ raw2ppm.c

frm2raw6: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=6 -o $@ frm2raw.c

ppm2frm6: ppm2frm.c colors4s1.h distance4s1.h
	$(CC) $(CFLAGS) -DMODE=6 -o $@ ppm2frm.c

raw2ppm7: raw2ppm.c palette4.h
	$(CC) $(CFLAGS) -DMODE=7 -DCOLORSET=2 -o $@ raw2ppm.c

frm2raw7: frm2raw.c
	$(CC) $(CFLAGS) -DMODE=7 -o $@ frm2raw.c

ppm2frm7: ppm2frm.c colors4a.h distance4a.h
	$(CC) $(CFLAGS) -DMODE=7 -o $@ ppm2frm.c
