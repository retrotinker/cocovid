#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdint.h>

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 5 + 3];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s inraw indrl outfile\n", prg);
}

void writerun(unsigned char buffer[], unsigned char val, int len)
{
	int i;

	for (i=0; i<len; i++)
		buffer[i] = val;
}

int main(int argc, char *argv[])
{
	int prevfd, curfd, outfd;
	unsigned char *inptr, *outptr;
	int insize, runsize;
	int rc;

	if (argc < 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open previous file */
	prevfd = open(argv[1], O_RDONLY);

	/* open current file */
	curfd = open(argv[2], O_RDONLY);

	/* open output file */
	outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	insize = 0;
	do {
		rc = read(prevfd, outbuf+insize, sizeof(outbuf)-insize);
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	insize = 0;
	do {
		rc = read(curfd, inbuf+insize, sizeof(inbuf)-insize);
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	/* strip end of frame marker */
	insize -= 3;

	inptr = inbuf;
	outptr = outbuf;

	for (inptr = inbuf; inptr < inbuf + insize; inptr++) {
		if (outptr - outbuf > sizeof(outbuf)) {
			printf("Frame offset out of bounds: %s %zd\n",
				argv[2], (size_t)outptr - (size_t)outbuf);
			break;
		}
		if (*inptr == 0xf0) {
			outptr = &outbuf[(*(inptr + 1) << 8) + *(inptr + 2)];
			inptr += 2;
			continue;
		}
		if ((*inptr & 0xf0) == 0xf0) {
			runsize = *inptr & 0x0f;
			writerun(outptr, *(inptr + 1), runsize);
			inptr += 1;
			outptr += runsize;
			continue;
		}
		*outptr++ = *inptr;
	}

	if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
		perror("pixel write");

	close(prevfd);
	close(curfd);
	close(outfd);

	return 0;
}
