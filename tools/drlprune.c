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

struct rlerun {
	unsigned char *start;
	unsigned int offset;
	unsigned int runlen, vidlen;
	unsigned int score;
};

unsigned char inbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 + 3];
unsigned char outbuf[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 * 5 + 3];

/* Assume we won't have separate runs that are physically adjacent... */
struct rlerun runpool[RAW_VERT_PIXELS * RAW_HORIZ_PIXELS/2 / 2];

int numrleruns = 0;

int cmprlerun(const void *run1, const void *run2)
{
	const struct rlerun *r1, *r2;

	r1 = run1;
	r2 = run2;

	if (r1->vidlen > r2->vidlen)
		return -1;
	else if (r1->vidlen == r2->vidlen && r1->score < r2->score)
		return -1;
	else if (r1->vidlen == r2->vidlen && r1->score == r2->score)
		return 0;
	else
		return 1;
}

void usage(char *prg)
{
	printf("Usage: %s indrl outdrl score\n", prg);
}

int rlevidlen(unsigned char *inbuf, int bufsize)
{
	int i, skip = 0, vidlen = 0;

	for (i=0; i<bufsize; i++) {
		if (skip) {
			skip--;
			continue;
		}
		if ((inbuf[i] & 0xc0) == 0xc0) {
			vidlen += (inbuf[i] & 0x3f);
			skip += 1;
		} else {
			vidlen += 1;
		}
	}

	return vidlen;
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

int rleprune(struct rlerun *run, int maxscore)
{
	int len;

	len = run->runlen;
	while (rlescore(run->start, len) > maxscore)
		len--;

	return len;
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int curoff = 0, insize = 0, outsize = 0;
	int maxscore, curscore = 0;
	int i, j, rc;

	if (argc < 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open previous file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (sscanf(argv[3], "%d", &maxscore) < 0) {
		perror("sscanf maxscore");
		exit(EXIT_FAILURE);
	}

	do {
		rc = read(infd, inbuf, sizeof(inbuf));
		if (rc < 0 && rc != EINTR) {
			perror("current read");
			exit(EXIT_FAILURE);
		}
		if (rc != EINTR)
			insize += rc;
	} while (rc != 0);

	/* strip end of frame marker */
	insize -= 3;

	i=0;
	if (inbuf[i] != 0xc0) {
		runpool[0].start = inbuf;
		runpool[0].offset = 0;
		for (j=i; j<insize; j++) {
			if (((inbuf[j] & 0xc0) == 0xc0) && (inbuf[j] != 0xc0))
				j += 1;
			else if (inbuf[j] == 0xc0)
				break;
		}
		runpool[0].runlen = j - i;
		runpool[0].vidlen = rlevidlen(&inbuf[i], runpool[0].runlen);
		runpool[0].score = 3 + rlescore(&inbuf[i], runpool[0].runlen);
		numrleruns = 1;
		i = j;
	}
	while (i < insize) {
		runpool[numrleruns].start = inbuf + i + 3;
		runpool[numrleruns].offset = (inbuf[i + 1] << 8) + inbuf[i + 2];
		for (j=i+3; j<insize; j++)
			if (((inbuf[j] & 0xc0) == 0xc0) && (inbuf[j] != 0xc0))
				j += 1;
			else if (inbuf[j] == 0xc0)
				break;
		if (runpool[numrleruns].offset == 0x1800) {
			i = j;
			break;
		}
		runpool[numrleruns].runlen = j - i - 3;
		runpool[numrleruns].vidlen = rlevidlen(&inbuf[i+3], runpool[numrleruns].runlen);
		runpool[numrleruns].score = 3 + rlescore(&inbuf[i+3], runpool[numrleruns].runlen);
		numrleruns += 1;
		i = j;
	}

	qsort(runpool, numrleruns, sizeof(struct rlerun), cmprlerun);

#ifdef DUMPRUNS
	for (i=0; i<numrleruns; i++) {
		printf("%4d: %04x %04x %04x %5d %5d\n",
			i, runpool[i].start - inbuf, runpool[i].offset,
			runpool[i].runlen, runpool[i].vidlen, runpool[i].score);
	}
#endif

	for (i=0; i<numrleruns; i++) {
		if (curscore + runpool[i].score < maxscore - 3) {
			outbuf[outsize++] = 0xc0;
			outbuf[outsize++] = (runpool[i].offset & 0xff00) >> 8;
			outbuf[outsize++] = runpool[i].offset & 0x00ff;
			for (j=0; j<runpool[i].runlen; j++) {
				outbuf[outsize++] = runpool[i].start[j];
			}
			curscore += runpool[i].score;
			curoff = runpool[i].offset + runpool[i].vidlen;

			/* mark this run as used */
			runpool[i].offset = 0xffff;
		}
	}

#if 0
	/*
	 * "Corporate" budgeting (i.e. use it or lose it)
	 * 	-- check for left-over budget
	 *	-- find largest unused run (guaranteed too big)
	 *	-- use rleprune to determine shorter length
	 *	-- copy start of run to output
	 */
	if (maxscore - curscore >= 8) {
		int prunelen;

		for (i=0; i<numrleruns; i++)
			if (runpool[i].offset != 0xffff)
				break;

		prunelen = rleprune(&runpool[i], maxscore - curscore);
		if (prunelen) {
			outbuf[outsize++] = 0xc0;
			outbuf[outsize++] = (runpool[i].offset & 0xff00) >> 8;
			outbuf[outsize++] = runpool[i].offset & 0x00ff;
			for (j=0; j<prunelen; j++) {
				outbuf[outsize++] = runpool[i].start[j];
			}
			curscore += runpool[i].score; /* not really needed here */
			curoff = runpool[i].offset +
					rlevidlen(runpool[i].start, prunelen);
		}
	}
#endif

	outbuf[outsize++] = 0xc0;
	outbuf[outsize++] = 0xff;
	outbuf[outsize++] = 0xff;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(infd);
	close(outfd);

	return 0;
}
