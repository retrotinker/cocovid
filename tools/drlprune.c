#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "distance.h"

#define RAW_HORIZ_PIXELS	256
#define RAW_VERT_PIXELS		192

#define PIXELS_PER_BYTE		2

/*
 * Account for run data plus 3-byte run header; needs to be short enough
 * to avoid distracting visual effects...
 */
#define MAXRUNLEN		(RAW_HORIZ_PIXELS / (4 * PIXELS_PER_BYTE))

struct vidrun {
	unsigned char *data;
	unsigned int rasterlen;
	unsigned int datalen;
	unsigned int offset;
	unsigned int colordiff;
	unsigned int adjscore;
};

struct splitrun {
	struct splitrun *next;
	unsigned char data[2];
};
struct splitrun *splitrun_head = NULL;

unsigned char prevbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];
unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 5 + 3];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 5 + 3];

struct vidrun runpool[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2];

int compare_vidruns(const void *run1, const void *run2)
{
	const struct vidrun *r1, *r2;

	r1 = run1;
	r2 = run2;

	if (r1->colordiff > r2->colordiff)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->rasterlen > r2->rasterlen)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->rasterlen == r2->rasterlen &&
			r1->datalen < r2->datalen)
		return -1;
	else if (r1->colordiff == r2->colordiff &&
			r1->rasterlen == r2->rasterlen &&
			r1->datalen == r2->datalen)
		return 0;
	else
		return 1;
}

void usage(char *prg)
{
	printf("Usage: %s prevraw indrl outdrl score\n", prg);
}

int main(int argc, char *argv[])
{
	int prevfd, infd, outfd;
	int insize, outsize = 0;
	int maxscore, curscore = 0;
	int current, offset = 0;
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
	insize -= 3;

	/* hack to correctly allocate first run */
	if (inbuf[0] == 0xf0)
		current = -1;
	else {
		current = 0;
		runpool[0].data = &inbuf[0];
		runpool[0].offset = 0;
		runpool[0].rasterlen = 0;
		runpool[0].datalen = 3;
	}

	/* Scan input to identify runs... */
	for (i = 0; i < insize; i++) {
		if (inbuf[i] == 0xf0) {
			/* start new run */
			current++;
			offset = (inbuf[i+1] << 8) + inbuf[i+2];
			runpool[current].data = &inbuf[i+3];
			runpool[current].offset = offset;
			runpool[current].datalen = 3;
			i += 2;
			continue;
		}
		if (runpool[current].datalen >= MAXRUNLEN ||
		    ((inbuf[i] & 0xf0) == 0xf0 &&
		     runpool[current].datalen + (inbuf[i] & 0x0f) > MAXRUNLEN)) {
			/* artificially start new run */
			current++;
			runpool[current].data = &inbuf[i];
			runpool[current].offset = offset;
			runpool[current].datalen = 3;
		}
		if ((inbuf[i] & 0xf0) == 0xf0) {
			/* Split RLE runs if longer than MAXRUNLEN... */
			while ((inbuf[i] & 0x0f) > MAXRUNLEN) {
				uint8_t oldlen, len;
				struct splitrun *run;
				unsigned char *buf;

				run = malloc(sizeof(struct splitrun));
				if (!run) {
					perror("malloc");
					exit(EXIT_FAILURE);
				} else {
					run->next = splitrun_head;
					splitrun_head = run;
					buf = &run->data[0];
				}

				oldlen = inbuf[i] & 0x0f;
				len = oldlen < MAXRUNLEN*2 ?
					oldlen / 2 : MAXRUNLEN;
				inbuf[i] -= len;

				buf[0] = 0xf0 | len;
				buf[1] = inbuf[i + 1];

				runpool[current].data = buf;
				runpool[current].rasterlen = len;
				runpool[current].datalen = 5;

				/* compute color difference for RLE run */
				for (j = 0; j < len; j++) {
					runpool[current].colordiff +=
						distance[inbuf[i+1] >> 8][prevbuf[offset+j] >> 8];
					runpool[current].colordiff +=
						distance[inbuf[i+1] & 0x0f][prevbuf[offset+j] & 0x0f];
				}

				/* add adjustment to score RLE runs properly */
				runpool[current].adjscore += len > 1 ?
					((len - 1) / 2) : 0;

				/* start another new run */
				offset += len;
				current++;
				runpool[current].data = &inbuf[i];
				runpool[current].offset = offset;
				runpool[current].datalen = 3;
			}
			runpool[current].rasterlen += inbuf[i] & 0x0f;
			runpool[current].datalen += 2;

			/* compute color difference for RLE run */
			for (j = 0; j < (inbuf[i] & 0x0f); j++) {
				runpool[current].colordiff +=
					distance[inbuf[i+1] >> 8][prevbuf[offset+j] >> 8];
				runpool[current].colordiff +=
					distance[inbuf[i+1] & 0x0f][prevbuf[offset+j] & 0x0f];
			}

			/* add adjustment to score RLE runs properly */
			runpool[current].adjscore += (inbuf[i] & 0x0f) > 1 ?
				(((inbuf[i] & 0x0f) - 1) / 2) : 0;

			offset += inbuf[i] & 0x0f;
			i += 1;
			continue;
		}
		runpool[current].rasterlen++;
		runpool[current].datalen++;
		runpool[current].colordiff +=
			distance[inbuf[i] >> 8][prevbuf[offset] >> 8];
		runpool[current].colordiff +=
			distance[inbuf[i] & 0x0f][prevbuf[offset] & 0x0f];
		offset++;
	}

	/* Sort according to above... */
	qsort(runpool, current + 1, sizeof(runpool[0]), compare_vidruns);

	/* Emit runs in sorted order until quota is fulfilled... */
	for (i = 0; i < current + 1; i++) {
		if (curscore + runpool[i].datalen + runpool[i].adjscore <
				maxscore - 3) {
			outbuf[outsize++] = 0xf0;
			outbuf[outsize++] = (runpool[i].offset & 0xff00) >> 8;
			outbuf[outsize++] = runpool[i].offset & 0x00ff;
			for (j=0; j<runpool[i].datalen - 3; j++) {
				outbuf[outsize++] = runpool[i].data[j];
			}
			curscore += runpool[i].datalen;
			curscore += runpool[i].adjscore;
		}
		/* Need room for EOF and minimum run or no point continuing */
		if (curscore >= maxscore - 7)
			break;
	}

	/* Always emit end of frame marker... */
	outbuf[outsize++] = 0xf0;
	outbuf[outsize++] = 0xff;
	outbuf[outsize++] = 0xff;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(prevfd);
	close(infd);
	close(outfd);

	while (splitrun_head) {
		struct splitrun *tmp;

		tmp = splitrun_head;
		splitrun_head = tmp->next;
		free(tmp);
	}

	return 0;
}
