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

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 5 + 3];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s inraw indrl outfile\n", prg);
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
	int prevfd, curfd, outfd;
	unsigned char *inptr, *outptr, *runstart;
	int outsize, runsize;
	int insize = 0;
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

	if (read(prevfd, &outbuf, sizeof(outbuf)) != sizeof(outbuf))
		perror("prev read");

	do {
		rc = read(curfd, inbuf, sizeof(inbuf));
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	inptr = inbuf;
	outptr = outbuf;
	runsize = 0;
	runstart = inptr;

	for (inptr = inbuf; inptr < inbuf + insize; inptr++) {
		if ((*inptr & 0xc0) == 0xc0) {
			if (*inptr != 0xc0) {
				inptr += 1;
				runsize +=2;
				continue;
			} else {
				if (*(inptr + 1) == 0xff &&
				    *(inptr + 2) == 0xff) {
					outptr = outbuf + 0x1800;
				} else {
					rledecompress(runstart, outptr,
							runsize);
					outptr = outbuf +
						(*(inptr + 1) << 8) +
						 *(inptr + 2);
				}
				inptr += 2;
				runstart = inptr + 1;
				runsize = 0;
				continue;
			}
		}
		runsize++;
	}

	outptr += rledecompress(runstart, outptr, runsize);
	outsize = outptr - outbuf;

	if (write(outfd, outbuf, outptr - outbuf) != outsize)
		perror("pixel write");

	if (outsize != sizeof(outbuf)) {
		printf("%s decompressed to wrong size! (%d)\n", argv[2], outsize);
		exit(EXIT_FAILURE);
	}

	close(prevfd);
	close(curfd);
	close(outfd);

	return 0;
}
