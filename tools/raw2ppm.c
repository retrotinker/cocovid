#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "palette.h"

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192

#define PPM_HEADER	"P6\n128 192\n255\n"
#define PPM_HEADER_SIZE	15

unsigned char inbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS];
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
		for (i=0; i<RAW_HORIZ_PIXELS; i++) {
			outbuf[j][i].r = palette[inbuf[j][i]].r;
			outbuf[j][i].g = palette[inbuf[j][i]].g;
			outbuf[j][i].b = palette[inbuf[j][i]].b;
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
