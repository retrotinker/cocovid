#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "colors.h"

#define PPM_PIXEL_OFFSET	0xE
#define PPM_HORIZ_PIXELS	128
#define PPM_VERT_PIXELS		96

struct rgb24 {
	uint8_t r, g, b;
} __attribute__ ((packed));

struct pixmap24 {
	struct rgb24 pixel[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS];
} __attribute__ ((packed));

struct pixmap24 inmap;

unsigned char cocobuf[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	char hdbuf[PPM_PIXEL_OFFSET];
	int i, j;
	int whitecount = 0;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while (whitecount < 4) {
		if (read(infd, hdbuf, 1) != 1)
			perror("head read");
		if (hdbuf[0] == '\n' || isblank(hdbuf[0])) {
			whitecount++;
			while (whitecount < 4 &&
				hdbuf[0] == '\n' || isblank(hdbuf[0])) {
				if (read(infd, hdbuf, 1) != 1)
					perror("head read");
				if (hdbuf[0] == '#')
					while (hdbuf[0] != '\n') {
						if (read(infd, hdbuf, 1) != 1)
							perror("head read");
					}
			}
		}
	}

	if (read(infd, &inmap, sizeof(inmap)) != sizeof(inmap))
		perror("pixel read");

	for (j=0; j<PPM_VERT_PIXELS; j++)
		for (i=0; i<PPM_HORIZ_PIXELS/2; i++) {
			unsigned char val;

			val = color[RGB(inmap.pixel[j][2*i].r,
					inmap.pixel[j][2*i].g,
					inmap.pixel[j][2*i].b)];
			val <<= 4;
			val |= color[RGB(inmap.pixel[j][2*i+1].r,
					 inmap.pixel[j][2*i+1].g,
					 inmap.pixel[j][2*i+1].b)];

			cocobuf[j][i] = val;
		}

	if (write(outfd, cocobuf, sizeof(cocobuf)) != sizeof(cocobuf))
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
