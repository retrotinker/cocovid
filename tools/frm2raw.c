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

#if defined(MODE)
#if MODE == 0
#define RAW_VERT_PIXELS		192
#elif MODE == 1
#define RAW_VERT_PIXELS		96
#endif
#else
#error "Unknown MODE value!"
#endif

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 2 + 2];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];

void usage(char *prg)
{
	printf("Usage: %s inraw infrm outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int prevfd, curfd, outfd;
	unsigned char *inptr, *outptr;
	int insize;
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
	insize -= 2;

	inptr = inbuf;
	outptr = outbuf;

	while (inptr < inbuf + insize) {
		int i, count = (*inptr & 0x07) + 1;

		/* jump to proper output offset */
		outptr = outbuf + ((*inptr & 0xf8) << 6) + (*(inptr + 1) << 1);
		inptr += 2;

		/* emit count words of video data */
		for (i = 0; i < count; i++) {
			*outptr++ = *inptr++;
			*outptr++ = *inptr++;
		}
	}

	if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
		perror("pixel write");

	close(prevfd);
	close(curfd);
	close(outfd);

	return 0;
}
