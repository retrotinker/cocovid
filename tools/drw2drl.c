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

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS * 5 + 3];
unsigned char tmpbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS * 2];

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int rlecompress(unsigned char *inbuf, unsigned char *outbuf, int bufsize)
{
	int i, count = 0, size = 0;
	unsigned char cur;

	if (bufsize < 1)
		return 0;

	cur = inbuf[0];
	if ((cur & 0xf0) == 0xf0) { /* check for reserved char */
		/* start counting */
		count++; /* just current */
	}
	for (i=1; i<bufsize; i++) {
		if (count) {
			if (cur == inbuf[i]) { /* check for continued match */
				count++; /* count */
				if (count > 0x0f) { /* check for max count */
					outbuf[size++] = 0xff;
					outbuf[size++] = cur;
					count = 1; /* reset count to one */
				}
			} else { /* end match */
				outbuf[size++] = 0xf0 | count;
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
		if (!count && (cur & 0xf0) == 0xf0) { /* check for reserved char */
			/* start counting */
			count++; /* just current */
		}
	}
	if (count)
		outbuf[size++] = 0xf0 | count;
	outbuf[size++] = cur;

	return size;
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int outsize, insize = 0;
	unsigned char *inptr, *tmpptr, *outptr;
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

	do {
		rc = read(infd, inbuf, sizeof(inbuf));
		if (rc < 0 && rc != EINTR) {
			perror("pixel read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	inptr = inbuf;
	tmpptr = tmpbuf;
	outptr = outbuf;

	while (inptr < inbuf + insize) {
		if (*inptr == 0xf0) {
			outptr += rlecompress(tmpbuf, outptr, tmpptr - tmpbuf);
			tmpptr = tmpbuf;
			*outptr++ = *inptr++;
			*outptr++ = *inptr++;
			*outptr++ = *inptr++;
		} else {
			if (*inptr == 0xf1) {
				*tmpptr++ = *(inptr + 1);
				inptr += 2;
			} else {
				*tmpptr++ = *inptr++;
			}
		}
	}
	outptr += rlecompress(tmpbuf, outptr, tmpptr - tmpbuf);
	outsize = outptr - outbuf;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
