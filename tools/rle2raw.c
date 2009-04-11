#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96

unsigned char inbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];
unsigned char outbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int rledecompress(unsigned char *inbuf, unsigned char *outbuf, int bufsize)
{
	int i, j, count = 0, size = 0;
	unsigned char cur, prev = 0;

	if (bufsize < 1)
		return 0;

	cur = inbuf[0];
	for (i=1; i<bufsize; i++) {
		if ((prev & 0xc0) == 0xc0) {
			prev = 0;
			cur = inbuf[i];
			continue;
		}
		if ((cur & 0xc0) == 0xc0) {
			count = cur & 0x3f;
			for (j=0; j<count; j++)
				outbuf[size++] = inbuf[i];
		} else {
			outbuf[size++] = cur;
		}
		prev = cur;
		cur = inbuf[i];
	}
	if ((prev & 0xc0) != 0xc0)
		outbuf[size++] = cur;

	return size;
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int insize, outsize;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if ((insize = read(infd, &inbuf, sizeof(inbuf))) < 0)
		perror("pixel read");

	outsize = rledecompress(&inbuf[0][0], &outbuf[0][0], insize);

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	if (outsize != sizeof(inbuf)) {
		printf("%s decompressed to wrong size! (%d)\n", argv[1], outsize);
		exit(EXIT_FAILURE);
	}

	close(infd);
	close(outfd);

	return 0;
}
