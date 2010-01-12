.PHONY: all

all: ppm2raw raw2ppm raw2rle raw2rli rle2raw raw2drw drw2drl drl2raw wav2pcm \
	pcm2clw clw2pcm drlscore drlprune

gencolors: gencolors.c palette.h
	$(CC) -o gencolors gencolors.c -lm

colors.h: gencolors
	./gencolors > $@

distance.h: gencolors
	./gencolors -d > $@

ppm2raw: ppm2raw.c colors.h
	$(CC) -o ppm2raw ppm2raw.c

raw2ppm: raw2ppm.c palette.h
	$(CC) -o raw2ppm raw2ppm.c

raw2rle: raw2rle.c
	$(CC) -o raw2rle raw2rle.c

raw2rli: raw2rli.c
	$(CC) -o raw2rli raw2rli.c

rle2raw: rle2raw.c
	$(CC) -o rle2raw rle2raw.c

raw2drw: raw2drw.c
	$(CC) -o raw2drw raw2drw.c

drw2drl: drw2drl.c
	$(CC) -o drw2drl drw2drl.c

drl2raw: drl2raw.c
	$(CC) -o drl2raw drl2raw.c

wav2pcm: wav2pcm.c
	$(CC) -o wav2pcm wav2pcm.c

pcm2clw: pcm2clw.c
	$(CC) -o pcm2clw pcm2clw.c

clw2pcm: clw2pcm.c
	$(CC) -o clw2pcm clw2pcm.c

drlscore: drlscore.c
	$(CC) -o drlscore drlscore.c

drlprune: drlprune.c distance.h
	$(CC) -o drlprune drlprune.c
