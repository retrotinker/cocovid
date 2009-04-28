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

void usage(char *prg)
{
	printf("Usage: %s indrl\n", prg);
}

int rlescore(unsigned char *inbuf, int bufsize)
{
	int i, skip = 0, score = 0;

	for (i=0; i<bufsize; i++) {
		if (skip) {
			skip--;
			continue;
		}
		if (inbuf[i] == 0xc0) {
			score += 3;
			skip += 2;
		} else if ((inbuf[i] & 0xc0) == 0xc0) {
			score += (2 + (inbuf[i] & 0x3f));
			skip += 1;
		} else {
			score += 2;
		}
	}

	return score;
}

int main(int argc, char *argv[])
{
	int infd;
	int insize = 0;
	int rc;

	if (argc < 2) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open previous file */
	infd = open(argv[1], O_RDONLY);

	do {
		rc = read(infd, inbuf, sizeof(inbuf));
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	printf("%d\n", rlescore(inbuf, insize));

	close(infd);

	return 0;
}
