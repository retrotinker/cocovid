/*
 * Copyright (c) 2009-2011, John W. Linville <linville@tuxdriver.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

#if defined(MODE)
#if MODE == 0
#include "colors16.h"
#include "distance16.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		2
#elif MODE == 1
#include "colors16.h"
#include "distance16.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		2
#elif MODE == 2
#include "colors256.h"
#include "distance256.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		1
#elif MODE == 3
#include "distance2.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		8
#elif MODE == 4
#include "distance2.h"
#define RAW_HORIZ_PIXELS	256
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		8
#elif MODE == 5
#include "colors4s0.h"
#include "distance4s0.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		4
#elif MODE == 6
#include "colors4s1.h"
#include "distance4s1.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		4
#elif MODE == 7
#include "colors4a.h"
#include "distance4a.h"
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		4
#endif
#else
#error "Unknown MODE value!"
#endif

#define BYTES_PER_READ		2
#define MAX_READS_PER_RUN	8
#define MAX_BYTES_PER_RUN	(MAX_READS_PER_RUN * BYTES_PER_READ)

struct rgb24 {
	uint8_t r, g, b;
} __attribute__ ((packed));

struct pixmap24 {
	struct rgb24 pixel[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS];
} __attribute__ ((packed));

struct vidrun {
	unsigned char *data;
	unsigned int datalen;
	unsigned int offset;
	unsigned int colordiff;
	unsigned int adjscore;
};

struct pixmap24 curmap;

unsigned char curraw[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS / PIXELS_PER_BYTE];
unsigned char prevraw[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS / PIXELS_PER_BYTE];

unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS / PIXELS_PER_BYTE * 2 + 2];

struct vidrun runpool[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS / PIXELS_PER_BYTE];
int currun = -1;

void usage(char *prg)
{
	printf("Usage: %s curppm prevraw score outfile\n", prg);
}

void ppm2raw(void)
{
	int i, j;

	for (i = 0; i < RAW_VERT_PIXELS; i++)
		for (j = 0; j < RAW_HORIZ_PIXELS / PIXELS_PER_BYTE; j++) {
#if PIXELS_PER_BYTE == 2
			unsigned char val;

			val = color[RGB(curmap.pixel[i][2*j].r,
					curmap.pixel[i][2*j].g,
					curmap.pixel[i][2*j].b)];
			val <<= 4;
			val |= color[RGB(curmap.pixel[i][2*j+1].r,
					 curmap.pixel[i][2*j+1].g,
					 curmap.pixel[i][2*j+1].b)];

			curraw[i][j] = val;
#elif PIXELS_PER_BYTE == 1
			curraw[i][j] = color[RGB(curmap.pixel[i][j].r,
							curmap.pixel[i][j].g,
							curmap.pixel[i][j].b)];
#elif PIXELS_PER_BYTE == 8
			unsigned char val = 0;

			if ((curmap.pixel[i][8*j].r) ||
			    (curmap.pixel[i][8*j].g) ||
			    (curmap.pixel[i][8*j].g))
				val |= 0x80;
			if ((curmap.pixel[i][8*j+1].r) ||
			    (curmap.pixel[i][8*j+1].g) ||
			    (curmap.pixel[i][8*j+1].g))
				val |= 0x40;
			if ((curmap.pixel[i][8*j+2].r) ||
			    (curmap.pixel[i][8*j+2].g) ||
			    (curmap.pixel[i][8*j+2].g))
				val |= 0x20;
			if ((curmap.pixel[i][8*j+3].r) ||
			    (curmap.pixel[i][8*j+3].g) ||
			    (curmap.pixel[i][8*j+3].g))
				val |= 0x10;
			if ((curmap.pixel[i][8*j+4].r) ||
			    (curmap.pixel[i][8*j+4].g) ||
			    (curmap.pixel[i][8*j+4].g))
				val |= 0x08;
			if ((curmap.pixel[i][8*j+5].r) ||
			    (curmap.pixel[i][8*j+5].g) ||
			    (curmap.pixel[i][8*j+5].g))
				val |= 0x04;
			if ((curmap.pixel[i][8*j+6].r) ||
			    (curmap.pixel[i][8*j+6].g) ||
			    (curmap.pixel[i][8*j+6].g))
				val |= 0x02;
			if ((curmap.pixel[i][8*j+7].r) ||
			    (curmap.pixel[i][8*j+7].g) ||
			    (curmap.pixel[i][8*j+7].g))
				val |= 0x01;

			curraw[i][j] = val;
#elif PIXELS_PER_BYTE == 4
			unsigned char val;

			val = color[RGB(curmap.pixel[i][4*j].r,
					curmap.pixel[i][4*j].g,
					curmap.pixel[i][4*j].b)];
			val <<= 2;
			val |= color[RGB(curmap.pixel[i][4*j+1].r,
					 curmap.pixel[i][4*j+1].g,
					 curmap.pixel[i][4*j+1].b)];
			val <<= 2;
			val |= color[RGB(curmap.pixel[i][4*j+2].r,
					 curmap.pixel[i][4*j+2].g,
					 curmap.pixel[i][4*j+2].b)];
			val <<= 2;
			val |= color[RGB(curmap.pixel[i][4*j+3].r,
					 curmap.pixel[i][4*j+3].g,
					 curmap.pixel[i][4*j+3].b)];

			curraw[i][j] = val;
#else
#error "Unknown PIXELS_PER_BYTE value!"
#endif
		}
}

void raw2runs(void)
{
	int i, j;
	int active = 0;

	for (i = 0; i < RAW_VERT_PIXELS; i++)
		for (j = 0; j < RAW_HORIZ_PIXELS / PIXELS_PER_BYTE;
			j += BYTES_PER_READ)
		{
                        int diff;

                        diff = prevraw[i][j] != curraw[i][j] ||
                                prevraw[i][j+1] != curraw[i][j+1];

			if ((active && !diff) ||
			    runpool[currun].datalen == 2 + MAX_BYTES_PER_RUN)
				active = 0;

			if (!active && diff) {
				/* allocate/start new run */
				currun++;
				runpool[currun].data = &curraw[i][j];
				runpool[currun].offset =
					(i * (RAW_HORIZ_PIXELS / PIXELS_PER_BYTE)) + j;
				runpool[currun].datalen = 2;
				runpool[currun].adjscore = 2;

				/* start recording data words */
				active = 1;
			}

			if (active) {
				/* update colordiff accumulation and data length */
#if PIXELS_PER_BYTE == 2
				runpool[currun].colordiff +=
					distance[curraw[i][j] >> 4][prevraw[i][j] >> 4];
				runpool[currun].colordiff +=
					distance[curraw[i][j] & 0x0f][prevraw[i][j] & 0x0f];
#elif PIXELS_PER_BYTE == 1
				runpool[currun].colordiff +=
					distance[curraw[i][j]][prevraw[i][j]];
#elif PIXELS_PER_BYTE == 8
				runpool[currun].colordiff +=
					distance[curraw[i][j] ^ prevraw[i][j]];
#elif PIXELS_PER_BYTE == 4
				runpool[currun].colordiff +=
					distance[(curraw[i][j] & 0xc0) >> 6][(prevraw[i][j] & 0xc0) >> 6];
				runpool[currun].colordiff +=
					distance[(curraw[i][j] & 0x30) >> 4][(prevraw[i][j] & 0x30) >> 4];
				runpool[currun].colordiff +=
					distance[(curraw[i][j] & 0x0c) >> 2][(prevraw[i][j] & 0x0c) >> 2];
				runpool[currun].colordiff +=
					distance[curraw[i][j] & 0x03][prevraw[i][j] & 0x03];
#else
#error "Unknown PIXELS_PER_BYTE value!"
#endif
				runpool[currun].datalen += 2;
			}
		}
}

int compare_vidruns(const void *run1, const void *run2)
{
	const struct vidrun *r1, *r2;

	r1 = run1;
	r2 = run2;

	if (r1->colordiff > r2->colordiff)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->datalen < r2->datalen)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->datalen == r2->datalen)
		return 0;
	else
		return 1;
}

int main(int argc, char *argv[])
{
	int ppmfd, rawfd, outfd;
	char hdbuf;
	int insize, outsize = 0;
	int whitecount = 0;
	int maxscore, curscore = 0;
	int i, j, rc;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open current ppm file */
	if (!strncmp(argv[1], "-", 1))
		ppmfd = 0;
	else
		ppmfd = open(argv[1], O_RDONLY);

	/* open previous raw file */
	if (!strncmp(argv[2], "-", 1))
		rawfd = 0;
	else
		rawfd = open(argv[2], O_RDONLY);

	/* read max score per frame */
	if (sscanf(argv[3], "%d", &maxscore) < 0) {
		perror("sscanf maxscore");
		exit(EXIT_FAILURE);
	}

	/* open output file */
	if (!strncmp(argv[4], "-", 1))
		outfd = 1;
	else
		outfd = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while (whitecount < 4) {
		if (read(ppmfd, &hdbuf, 1) != 1)
			perror("head read");
		if (hdbuf == '\n' || isblank(hdbuf)) {
			whitecount++;
			while ((whitecount < 4) &&
				(hdbuf == '\n' || isblank(hdbuf))) {
				if (read(ppmfd, &hdbuf, 1) != 1)
					perror("ppm head read");
				if (hdbuf == '#')
					while (hdbuf != '\n') {
						if (read(ppmfd, &hdbuf, 1) != 1)
							perror("ppm head read");
					}
			}
		}
	}

	insize = 0;
	do {
		rc = read(ppmfd, (char *)&curmap+insize, sizeof(curmap)-insize);
		if (rc < 0 && rc != EINTR) {
			perror("ppm data read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);
	close(ppmfd);

	insize = 0;
	do {
		rc = read(rawfd, (char *)&prevraw+insize, sizeof(prevraw)-insize);
		if (rc < 0 && rc != EINTR) {
			perror("prev raw data read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);
	close(rawfd);

	/* Convert current ppm to raw format... */
	ppm2raw();

	/* Find the differing runs between current and prev... */
	raw2runs();

	/* Sort runs by importance... */
	qsort(runpool, currun + 1, sizeof(runpool[0]), compare_vidruns);

	/* Emit runs in sorted order until quota is fulfilled... */
	for (i = 0; i <= currun; i++) {
		if (curscore + runpool[i].datalen + runpool[i].adjscore <
				maxscore - 2) {
			outbuf[outsize++] =
				((runpool[i].offset >> 1) & 0x1fe0) >> 5;
			outbuf[outsize++] =
				(((runpool[i].offset >> 1) & 0x001f) << 3) |
					(((runpool[i].datalen - 2) /
						BYTES_PER_READ) - 1);
			for (j = 0; j < runpool[i].datalen - 2; j++) {
				outbuf[outsize++] = runpool[i].data[j];
			}
			curscore += runpool[i].datalen;
			curscore += runpool[i].adjscore;
		}
		/* Need room for EOF and minimum run or no point continuing */
		if (curscore > maxscore - 6)
			break;
	}

	/* Always emit end of frame marker... */
	outbuf[outsize++] = 0xff;
	outbuf[outsize++] = 0x00;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("frame write");

	close(outfd);

	return 0;
}
