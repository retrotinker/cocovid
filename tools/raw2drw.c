#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		96

unsigned char prevbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];
unsigned char curbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];

/* Quintuple the output buffer in case of degenerate worst case */
unsigned char outbuf[RAW_VERT_PIXELS*(RAW_HORIZ_PIXELS/2)*5];

void usage(char *prg)
{
	printf("Usage: %s prevfile curfile outfile\n", prg);
}

int badmatch(unsigned char *prev, unsigned char *cur, int offset)
{
	int i;

	for (i=0; i<4; i++)
		if (prev[i] != cur[i])
			break;

	if (i == 4 && offset + 5 < 0x1800)
		return 0;
	else if (i == 3 && cur[3] != 0xc0 &&
	         offset + 4 < 0x1800)
		return 0;

	return 1;
}

int main(int argc, char *argv[])
{
	int prevfd, curfd, outfd;
	int i, j, matching = 0, outsize = 0;

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

	if (read(prevfd, &prevbuf, sizeof(prevbuf)) != sizeof(prevbuf))
		perror("prev read");

	if (read(curfd, &curbuf, sizeof(curbuf)) != sizeof(curbuf))
		perror("cur read");

	for (i = 0; i < RAW_VERT_PIXELS; i++)
		for (j = 0; j < (RAW_HORIZ_PIXELS/2); j++) {
			int offset = (i * (RAW_HORIZ_PIXELS/2)) + j;

			if (!matching && prevbuf[i][j] == curbuf[i][j] &&
			    !badmatch(&prevbuf[i][j], &curbuf[i][j], offset))
				matching = 1;
			else if (matching && prevbuf[i][j] != curbuf[i][j]) {
				outbuf[outsize++] = 0xc0;
				outbuf[outsize++] =
					(offset & 0xff00) >> 8;
				outbuf[outsize++] =
					offset & 0x00ff;
				matching = 0;
			}
			if (!matching) {
				if ((curbuf[i][j] == 0xc0) ||
				    (curbuf[i][j] == 0xc1))
					outbuf[outsize++] = 0xc1;
				outbuf[outsize++] = curbuf[i][j];
			}
		}

	/* Always have at least a frame delimeter...? */
	if (matching) {
		outbuf[outsize++] = 0xc0;
		outbuf[outsize++] = 0x18;
		outbuf[outsize++] = 0x00;
	}

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(prevfd);
	close(curfd);
	close(outfd);

	return 0;
}
