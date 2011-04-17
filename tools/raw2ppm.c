#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#if defined(MODE)
#if MODE == 0
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		2
#define PPM_HEADER	"P6\n128 192\n255\n"
#define PPM_HEADER_SIZE	15
#elif MODE == 1
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		2
#define PPM_HEADER	"P6\n128 96\n255\n"
#define PPM_HEADER_SIZE	14
#elif MODE == 2
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		1
#define PPM_HEADER	"P6\n128 96\n255\n"
#define PPM_HEADER_SIZE	14
#elif MODE == 3
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		8
#define PPM_HEADER	"P6\n128 192\n255\n"
#define PPM_HEADER_SIZE	15
#elif MODE == 4
#define RAW_HORIZ_PIXELS	256
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		8
#define PPM_HEADER	"P6\n256 192\n255\n"
#define PPM_HEADER_SIZE	15
#elif MODE == 5
#define RAW_HORIZ_PIXELS	512
#define RAW_VERT_PIXELS		192
#define PIXELS_PER_BYTE		8
#define PPM_HEADER	"P6\n512 192\n255\n"
#define PPM_HEADER_SIZE	15
#elif MODE == 6
#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96
#define PIXELS_PER_BYTE		4
#define PPM_HEADER	"P6\n128 96\n255\n"
#define PPM_HEADER_SIZE	14
#endif
#else
#error "Unknown MODE value!"
#endif

#if PIXELS_PER_BYTE == 1
#include "palette256.h"
#elif PIXELS_PER_BYTE == 2
#include "palette16.h"
#elif PIXELS_PER_BYTE == 8
#include "palette2.h"
#elif PIXELS_PER_BYTE == 4
#include "palette4.h"
#else
#error "Unknown PIXELS_PER_BYTE value!"
#endif

unsigned char inbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS / PIXELS_PER_BYTE];
struct rgb outbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int outsize, insize = 0;
	int i, j;
	int rc;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	do {
		rc = read(infd, inbuf, sizeof(inbuf));
		if (rc < 0 && rc != EINTR) {
			perror("pixel read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	/* process image data */
	for (j=0; j<RAW_VERT_PIXELS; j++)
		for (i=0; i<RAW_HORIZ_PIXELS / PIXELS_PER_BYTE; i++) {
#if PIXELS_PER_BYTE == 2
			outbuf[j][i*2].r = palette[(inbuf[j][i] & 0xf0) >> 4].r;
			outbuf[j][i*2].g = palette[(inbuf[j][i] & 0xf0) >> 4].g;
			outbuf[j][i*2].b = palette[(inbuf[j][i] & 0xf0) >> 4].b;
			outbuf[j][i*2+1].r = palette[inbuf[j][i] & 0x0f].r;
			outbuf[j][i*2+1].g = palette[inbuf[j][i] & 0x0f].g;
			outbuf[j][i*2+1].b = palette[inbuf[j][i] & 0x0f].b;
#elif PIXELS_PER_BYTE == 1
			outbuf[j][i].r = palette[inbuf[j][i]].r;
			outbuf[j][i].g = palette[inbuf[j][i]].g;
			outbuf[j][i].b = palette[inbuf[j][i]].b;
#elif PIXELS_PER_BYTE == 8
			outbuf[j][i*8].r = (inbuf[j][i] & 0x80) ? 0xff : 0x00;
			outbuf[j][i*8].g = (inbuf[j][i] & 0x80) ? 0xff : 0x00;
			outbuf[j][i*8].b = (inbuf[j][i] & 0x80) ? 0xff : 0x00;
			outbuf[j][i*8+1].r = (inbuf[j][i] & 0x40) ? 0xff : 0x00;
			outbuf[j][i*8+1].g = (inbuf[j][i] & 0x40) ? 0xff : 0x00;
			outbuf[j][i*8+1].b = (inbuf[j][i] & 0x40) ? 0xff : 0x00;
			outbuf[j][i*8+2].r = (inbuf[j][i] & 0x20) ? 0xff : 0x00;
			outbuf[j][i*8+2].g = (inbuf[j][i] & 0x20) ? 0xff : 0x00;
			outbuf[j][i*8+2].b = (inbuf[j][i] & 0x20) ? 0xff : 0x00;
			outbuf[j][i*8+3].r = (inbuf[j][i] & 0x10) ? 0xff : 0x00;
			outbuf[j][i*8+3].g = (inbuf[j][i] & 0x10) ? 0xff : 0x00;
			outbuf[j][i*8+3].b = (inbuf[j][i] & 0x10) ? 0xff : 0x00;
			outbuf[j][i*8+4].r = (inbuf[j][i] & 0x08) ? 0xff : 0x00;
			outbuf[j][i*8+4].g = (inbuf[j][i] & 0x08) ? 0xff : 0x00;
			outbuf[j][i*8+4].b = (inbuf[j][i] & 0x08) ? 0xff : 0x00;
			outbuf[j][i*8+5].r = (inbuf[j][i] & 0x04) ? 0xff : 0x00;
			outbuf[j][i*8+5].g = (inbuf[j][i] & 0x04) ? 0xff : 0x00;
			outbuf[j][i*8+5].b = (inbuf[j][i] & 0x04) ? 0xff : 0x00;
			outbuf[j][i*8+6].r = (inbuf[j][i] & 0x02) ? 0xff : 0x00;
			outbuf[j][i*8+6].g = (inbuf[j][i] & 0x02) ? 0xff : 0x00;
			outbuf[j][i*8+6].b = (inbuf[j][i] & 0x02) ? 0xff : 0x00;
			outbuf[j][i*8+7].r = (inbuf[j][i] & 0x01) ? 0xff : 0x00;
			outbuf[j][i*8+7].g = (inbuf[j][i] & 0x01) ? 0xff : 0x00;
			outbuf[j][i*8+7].b = (inbuf[j][i] & 0x01) ? 0xff : 0x00;
#elif PIXELS_PER_BYTE == 4
			outbuf[j][i*4].r = palette[(inbuf[j][i] & 0xc0) >> 6].r;
			outbuf[j][i*4].g = palette[(inbuf[j][i] & 0xc0) >> 6].g;
			outbuf[j][i*4].b = palette[(inbuf[j][i] & 0xc0) >> 6].b;
			outbuf[j][i*4+1].r = palette[(inbuf[j][i] & 0x30) >> 4].r;
			outbuf[j][i*4+1].g = palette[(inbuf[j][i] & 0x30) >> 4].g;
			outbuf[j][i*4+1].b = palette[(inbuf[j][i] & 0x30) >> 4].b;
			outbuf[j][i*4+2].r = palette[(inbuf[j][i] & 0x0c) >> 2].r;
			outbuf[j][i*4+2].g = palette[(inbuf[j][i] & 0x0c) >> 2].g;
			outbuf[j][i*4+2].b = palette[(inbuf[j][i] & 0x0c) >> 2].b;
			outbuf[j][i*4+3].r = palette[inbuf[j][i] & 0x03].r;
			outbuf[j][i*4+3].g = palette[inbuf[j][i] & 0x03].g;
			outbuf[j][i*4+3].b = palette[inbuf[j][i] & 0x03].b;
#else
#error "Unknown PIXELS_PER_BYTE value!"
#endif
		}

	/* output PPM header */
	if (write(outfd, PPM_HEADER, PPM_HEADER_SIZE) != PPM_HEADER_SIZE)
		perror("PPM header write");

	/* output image data */
	outsize = sizeof(outbuf);
	if (write(outfd, outbuf, outsize) != outsize)
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
