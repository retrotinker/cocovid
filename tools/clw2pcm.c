#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unsigned char clwbuf[512];
unsigned char outbuf[1024];

/*
 * Sequence: -32,-21,-13, -9, -6, -4, -2, -1,  0,  1,  3,  5,  8, 12, 20, 31
 * Unsigned:   0, 11, 19, 23, 26, 28, 30, 31, 32, 33, 35, 37, 40, 44, 52, 63
 *  Encoded:  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0a, 0b, 0c, 0d, 0e, 0f
 *  Decoded:  00, 0b, 13, 17, 1a, 1c, 1e, 1f, 20, 21, 23, 25, 28, 2c, 34, 3f
 *  Shifted:  00, 2c, 4c, 5c, 68, 70, 78, 7c, 80, 84, 8c, 94, a0, b0, d0, fc
 */
unsigned char pcmtbl[16] = {
	0x00, 0x2c, 0x4c, 0x5c, 0x68, 0x70, 0x78, 0x7c,
	0x80, 0x84, 0x8c, 0x94, 0xa0, 0xb0, 0xd0, 0xfc,
};

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int rc, val;
	int clwidx = 0, outidx = 0;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while ((rc = read(infd, &clwbuf, sizeof(clwbuf))) == sizeof(clwbuf)) {
		while (clwidx < sizeof(clwbuf)) {
			val = clwbuf[clwidx++];
			outbuf[outidx++] = pcmtbl[(val & 0xf0) >> 4] & 0xfc;
			outbuf[outidx++] = pcmtbl[val & 0x0f] & 0xfc;
		}
		if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
			perror("clw write");
		clwidx = 0;
		outidx = 0;
	}

	if (rc < 0)
		perror("clw read");
	else {
		while (clwidx < rc) {
			val = clwbuf[clwidx++];
			outbuf[outidx++] = pcmtbl[(val & 0xf0) >> 4] & 0xfc;
			outbuf[outidx++] = pcmtbl[val & 0x0f] & 0xfc;
		}
		if (write(outfd, outbuf, outidx) != outidx)
				perror("clw write");
	}

	close(infd);
	close(outfd);

	return 0;
}
