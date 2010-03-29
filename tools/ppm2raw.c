#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#if 0
#include "palette.h"
#endif
#include "colors.h"

#define PPM_HORIZ_PIXELS	128
#define PPM_VERT_PIXELS		192

struct rgb24 {
	uint8_t r, g, b;
} __attribute__ ((packed));

struct pixmap24 {
	struct rgb24 pixel[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS];
} __attribute__ ((packed));

struct pixmap24 inmap;

unsigned char cocobuf[PPM_VERT_PIXELS][PPM_HORIZ_PIXELS];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

inline uint8_t add_clamp(uint8_t a, int16_t b)
{
	int16_t tmp = a + b;

	if (tmp < 0)
		return 0;

	if (tmp > 255)
		return 255;

	return tmp;
}

#if 0
void dither(int h, int v, uint8_t color)
{
	int16_t r_error, g_error, b_error;

	/* Floyd–Steinberg dithering */
	r_error = inmap.pixel[v][h].r -
			palette[color].r;
	g_error = inmap.pixel[v][h].g -
			palette[color].g;
	b_error = inmap.pixel[v][h].b -
			palette[color].b;

	if (h < PPM_HORIZ_PIXELS-1) {
		inmap.pixel[v][h+1].r =
			add_clamp(inmap.pixel[v][h+1].r, ((7 * r_error) / 16));
		inmap.pixel[v][h+1].g =
			add_clamp(inmap.pixel[v][h+1].g, ((7 * g_error) / 16));
		inmap.pixel[v][h+1].b =
			add_clamp(inmap.pixel[v][h+1].b, ((7 * b_error) / 16));
	}

	if (v < PPM_VERT_PIXELS-1) {
		if (h > 0) {
			inmap.pixel[v+1][h-1].r =
				add_clamp(inmap.pixel[v+1][h-1].r,
						((3 * r_error) / 16));
			inmap.pixel[v+1][h-1].g =
				add_clamp(inmap.pixel[v+1][h-1].g,
						((3 * g_error) / 16));
			inmap.pixel[v+1][h-1].b =
				add_clamp(inmap.pixel[v+1][h-1].b,
						((3 * b_error) / 16));
		}
	
		inmap.pixel[v+1][h].r =
			add_clamp(inmap.pixel[v+1][h].r, ((5 * r_error) / 16));
		inmap.pixel[v+1][h].g =
			add_clamp(inmap.pixel[v+1][h].g, ((5 * g_error) / 16));
		inmap.pixel[v+1][h].b =
			add_clamp(inmap.pixel[v+1][h].b, ((5 * b_error) / 16));
	
		if (h < PPM_HORIZ_PIXELS-1) {
			inmap.pixel[v+1][h+1].r =
				add_clamp(inmap.pixel[v+1][h+1].r,
						((1 * r_error) / 16));
			inmap.pixel[v+1][h+1].g =
				add_clamp(inmap.pixel[v+1][h+1].g,
						((1 * g_error) / 16));
			inmap.pixel[v+1][h+1].b =
				add_clamp(inmap.pixel[v+1][h+1].b,
						((1 * b_error) / 16));
		}
	}
}
#endif

int main(int argc, char *argv[])
{
	int infd, outfd;
	char hdbuf;
	int i, j;
	int insize, whitecount = 0;
	int rc;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	if (!strncmp(argv[1], "-", 1))
		infd = 0;
	else
		infd = open(argv[1], O_RDONLY);

	/* open output file */
	if (!strncmp(argv[2], "-", 1))
		outfd = 1;
	else
		outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while (whitecount < 4) {
		if (read(infd, &hdbuf, 1) != 1)
			perror("head read");
		if (hdbuf == '\n' || isblank(hdbuf)) {
			whitecount++;
			while ((whitecount < 4) &&
				(hdbuf == '\n' || isblank(hdbuf))) {
				if (read(infd, &hdbuf, 1) != 1)
					perror("head read");
				if (hdbuf == '#')
					while (hdbuf != '\n') {
						if (read(infd, &hdbuf, 1) != 1)
							perror("head read");
					}
			}
		}
	}

	insize = 0;
	do {
		rc = read(infd, (char *)&inmap+insize, sizeof(inmap)-insize);
		if (rc < 0 && rc != EINTR) {
			perror("pixel read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);


	for (j=0; j<PPM_VERT_PIXELS; j++)
		for (i=0; i<PPM_HORIZ_PIXELS; i++) {
			cocobuf[j][i] = color[RGB(inmap.pixel[j][i].r,
							inmap.pixel[j][i].g,
							inmap.pixel[j][i].b)];
	}

	if (write(outfd, cocobuf, sizeof(cocobuf)) != sizeof(cocobuf))
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
