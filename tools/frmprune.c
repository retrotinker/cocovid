#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "distance.h"

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192

#define PIXELS_PER_BYTE		2

struct vidrun {
	unsigned char *data;
	unsigned int datalen;
	unsigned int offset;
	unsigned int colordiff;
	unsigned int adjscore;
};

unsigned char prevbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];
unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 2 + 2];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 2 + 2];

struct vidrun runpool[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];

int compare_vidruns(const void *run1, const void *run2)
{
	const struct vidrun *r1, *r2;

	r1 = run1;
	r2 = run2;

	if (r1->colordiff > r2->colordiff)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->datalen < r2->datalen)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->datalen == r2->datalen)
		return 0;
	else
		return 1;
}

void usage(char *prg)
{
	printf("Usage: %s prevraw infrm outfrm score\n", prg);
}

int main(int argc, char *argv[])
{
	int prevfd, infd, outfd;
	int insize, outsize = 0;
	int maxscore, curscore = 0;
	int current = -1;
	int i, j, rc;

	if (argc < 5) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open previous file */
	prevfd = open(argv[1], O_RDONLY);

	/* open input file */
	if (!strncmp(argv[2], "-", 1))
		infd = 0;
	else
		infd = open(argv[2], O_RDONLY);

	/* open output file */
	if (!strncmp(argv[2], "-", 1))
		outfd = 1;
	else
		outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (sscanf(argv[4], "%d", &maxscore) < 0) {
		perror("sscanf maxscore");
		exit(EXIT_FAILURE);
	}

	insize = 0;
	do {
		rc = read(prevfd, prevbuf, sizeof(prevbuf) - insize);
		if (rc < 0 && rc != EINTR) {
			perror("prev read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	insize = 0;
	do {
		rc = read(infd, inbuf, sizeof(inbuf) - insize);
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	/* strip end of frame marker */
	insize -= 2;

	/* Scan input to identify runs... */
	for (i = 0; i < insize; /* i is manipulated below */) {
		int offset, count;

		/* extract run information */
		offset = (((inbuf[i] & 0xf8) << 5) | inbuf[i+1]) << 1;
		count = (inbuf[i] & 0x7) + 1;

		/* allocate new run structure */
		current++;
		runpool[current].data = &inbuf[i+2];
		runpool[current].offset = offset;
		runpool[current].datalen = 2 + count * 2;
		runpool[current].adjscore = 2;

		/* walk the run to compute accumulated color diff */
		for (j = i + 2; j < i + 2 + count * 2; j++) {
			runpool[current].colordiff +=
				distance[inbuf[j] >> 4][prevbuf[offset] >> 4];
			runpool[current].colordiff +=
				distance[inbuf[j] & 0x0f][prevbuf[offset] & 0x0f];
		}

		/* manipulate i */
		i += runpool[current].datalen;
	}

	/* Sort according to above... */
	qsort(runpool, current + 1, sizeof(runpool[0]), compare_vidruns);

	/* Emit runs in sorted order until quota is fulfilled... */
	for (i = 0; i <= current; i++) {
		if (curscore + runpool[i].datalen + runpool[i].adjscore <
				maxscore - 2) {
			outbuf[outsize++] =
				(((runpool[i].offset >> 1) & 0x1f00) >> 5) |
					(((runpool[i].datalen - 2) / 2) - 1);
			outbuf[outsize++] = ((runpool[i].offset >> 1) & 0x00ff);
			for (j = 0; j < runpool[i].datalen - 2; j++) {
				outbuf[outsize++] = runpool[i].data[j];
			}
			curscore += runpool[i].datalen;
			curscore += runpool[i].adjscore;
		}
		/* Need room for EOF and minimum run or no point continuing */
		if (curscore > maxscore - 6)
			break;
	}

	/* Always emit end of frame marker... */
	outbuf[outsize++] = 0xff;
	outbuf[outsize++] = 0x00;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(prevfd);
	close(infd);
	close(outfd);

	return 0;
}
