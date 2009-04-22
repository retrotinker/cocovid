#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 2];

void usage(char *prg)
{
	printf("Usage: %s infile outfile sequence\n", prg);
}

int rlecompress(unsigned char *inbuf, unsigned char *outbuf, int bufsize)
{
	int i, count = 0, size = 0;
	unsigned char cur;

	if (bufsize < 1)
		return 0;

	cur = inbuf[0];
	if ((cur & 0xc0) == 0xc0) { /* check for reserved char */
		/* start counting */
		count++; /* just current */
	}
	for (i=1; i<bufsize; i++) {
		if (count) {
			if (cur == inbuf[i]) { /* check for continued match */
				count++; /* count */
				if (count > 0x3f) { /* check for max count */
					outbuf[size++] = 0xff;
					outbuf[size++] = cur;
					count = 1; /* reset count to one */
				}
			} else { /* end match */
				outbuf[size++] = 0xc0 | count;
				outbuf[size++] = cur;
				count = 0; /* reset count */
			}
		} else {
			if (cur == inbuf[i]) { /* check for new match */
				/* start counting */
				count = 2; /* current + next */
			} else {
				outbuf[size++] = cur;
			}
		}
		cur = inbuf[i];
		if (!count && (cur & 0xc0) == 0xc0) { /* check for reserved char */
			/* start counting */
			count++; /* just current */
		}
	}
	if (count)
		outbuf[size++] = 0xc0 | count;
	outbuf[size++] = cur;

	return size;
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int sequence;
	int output, outsize = 0, insize = 0;
	unsigned char *inptr, *outptr;
	int rc;

	if (argc < 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (sscanf(argv[3], "%d", &sequence) < 0) {
		perror("sscanf sequence");
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

	inptr = inbuf + (sequence - 1) * RAW_HORIZ_PIXELS/2;
	outptr = outbuf;
	while (insize > 0) {
		if (inptr != inbuf) {
			*outptr++ = 0xc0;
			*outptr++ = ((inptr - inbuf) & 0xff00) >> 8;
			*outptr++ = (inptr - inbuf) & 0x00ff;
			outsize += 3;
		}
		output = rlecompress(inptr, outptr, RAW_HORIZ_PIXELS/2);
		inptr += RAW_HORIZ_PIXELS/2 * 3;
		insize -= RAW_HORIZ_PIXELS/2 * 3;
		outptr += output;
		outsize += output;
	}
	if (inptr != (inbuf + sizeof(inbuf) +  RAW_HORIZ_PIXELS/2 * 2)) {
		*outptr++ = 0xc0;
		*outptr++ = 0x18;
		*outptr++ = 0x00;
		outsize += 3;
	}

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
