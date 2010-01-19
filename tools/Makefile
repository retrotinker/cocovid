.PHONY: all

CFLAGS=-Wall

all: ppm2raw raw2ppm raw2rle raw2drw drw2drl drl2raw wav2pcm \
	pcm2clw clw2pcm drlprune colormap.ppm

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

raw2rle: raw2rle.c
	$(CC) $(CFLAGS) -o raw2rle raw2rle.c

raw2drw: raw2drw.c
	$(CC) $(CFLAGS) -o raw2drw raw2drw.c

drw2drl: drw2drl.c
	$(CC) $(CFLAGS) -o drw2drl drw2drl.c

drlprune: drlprune.c distance.h
	$(CC) $(CFLAGS) -o drlprune drlprune.c

drl2raw: drl2raw.c
	$(CC) $(CFLAGS) -o drl2raw drl2raw.c

wav2pcm: wav2pcm.c
	$(CC) $(CFLAGS) -o wav2pcm wav2pcm.c

pcm2clw: pcm2clw.c
	$(CC) $(CFLAGS) -o pcm2clw pcm2clw.c

clw2pcm: clw2pcm.c
	$(CC) $(CFLAGS) -o clw2pcm clw2pcm.c
