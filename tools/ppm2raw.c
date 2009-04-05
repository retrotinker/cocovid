#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PPM_PIXEL_OFFSET	0xE
#define PPM_HORIZ_PIXELS	128
#define PPM_VERT_PIXELS		96

struct rgb24 {
	unsigned char r, g, b;
} __attribute__ ((packed));

struct pixmap24 {
	struct rgb24 pixel[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS];
} __attribute__ ((packed));

struct pixmap24 inmap;
struct pixmap24 outmap;

unsigned char cocobuf[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

void rlecompress(unsigned char *buf, int bufsize)
{
	int i, count = 0, size = 0;
	unsigned char cur;

	if (bufsize < 1) {
		printf("0\n");
		return;
	}

	for (i=0; i<bufsize-1; i++) {
		if (count) {
			if (buf[i] == cur) { /* check for continued match */
				/* count */
				count++;
			} else { /* end match */
				/* reset count */
				size++;
				count = 0;
			}
		} else {
			if (buf[i] & 0xc0) { /* check for reserved char */
				/* start counting */
				size++;
				count++;
			} else if (buf[i] == cur) { /* check for new match */
				/* start counting */
				size++;
				count++;
			} else { /* no match */
				size++;
			}
		}
		cur = buf[i];
	}
	size++;

	printf("%d\n", size);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	char hdbuf[PPM_PIXEL_OFFSET];
	int i, j;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (read(infd, hdbuf, sizeof(hdbuf)) != sizeof(hdbuf))
		perror("head read");

#if 0 /* ppm format */
	if (write(outfd, hdbuf, sizeof(hdbuf)) != sizeof(hdbuf))
		perror("head write");
#endif

	if (read(infd, &inmap, sizeof(inmap)) != sizeof(inmap))
		perror("pixel read");

	for (i=0; i<PPM_HORIZ_PIXELS; i++)
		for (j=0; j<PPM_VERT_PIXELS; j++) {
			outmap.pixel[j][i].r = inmap.pixel[j][i].r & 0xc0;
			outmap.pixel[j][i].g = inmap.pixel[j][i].g & 0xc0;
			outmap.pixel[j][i].b = inmap.pixel[j][i].b & 0xc0;
			if (((outmap.pixel[j][i].r & 0x40) &&
			     (outmap.pixel[j][i].g & 0x40)) ||
			    ((outmap.pixel[j][i].r & 0x40) &&
			     (outmap.pixel[j][i].b & 0x40)) ||
			    ((outmap.pixel[j][i].b & 0x40) &&
			     (outmap.pixel[j][i].g & 0x40))) {
				outmap.pixel[j][i].r |= 0x40;
				outmap.pixel[j][i].g |= 0x40;
				outmap.pixel[j][i].b |= 0x40;
			} else {
				outmap.pixel[j][i].r &= 0x80;
				outmap.pixel[j][i].g &= 0x80;
				outmap.pixel[j][i].b &= 0x80;
			}
		}

	for (j=0; j<PPM_VERT_PIXELS; j++)
		for (i=0; i<PPM_HORIZ_PIXELS/2; i++) {
			unsigned char val;

			val = (((outmap.pixel[j][2*i].r & 0x80) >> 1) |
				((outmap.pixel[j][2*i].g & 0x80) >> 2) |
				((outmap.pixel[j][2*i].b & 0x80) >> 3));
			if (outmap.pixel[j][2*i].r & 0x40)
				val |= 0x80;
			val |= (((outmap.pixel[j][2*i+1].r & 0x80) >> 5) |
				((outmap.pixel[j][2*i+1].g & 0x80) >> 6) |
				((outmap.pixel[j][2*i+1].b & 0x80) >> 7));
			if (outmap.pixel[j][2*i+1].r & 0x40)
				val |= 0x08;

			cocobuf[j][i] = val;
		}

#if 0 /* ppm format */
	if (write(outfd, &outmap, sizeof(outmap)) != sizeof(outmap))
		perror("pixel write");
#else /* coco vidbuf format */
	if (write(outfd, cocobuf, sizeof(cocobuf)) != sizeof(cocobuf))
		perror("pixel write");
#endif

	return 0;
}
