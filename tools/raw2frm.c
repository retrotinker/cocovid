#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RAW_HORIZ_PIXELS	128
#define RAW_VERT_PIXELS		192
#define SCREENEND		((RAW_HORIZ_PIXELS / 2) * RAW_VERT_PIXELS)

unsigned char prevbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];
unsigned char curbuf[RAW_VERT_PIXELS][RAW_HORIZ_PIXELS/2];

/* Quintuple the output buffer in case of degenerate worst case */
unsigned char outbuf[RAW_VERT_PIXELS*(RAW_HORIZ_PIXELS/2)*2 + 2];

void usage(char *prg)
{
	printf("Usage: %s prevfile curfile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int prevfd, curfd, outfd;
	int i, j, emitwords = 0, outsize = 0, emit = 0;

	if (argc < 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open previous file */
	prevfd = open(argv[1], O_RDONLY);

	/* open current file */
	if (!strncmp(argv[2], "-", 1))
		curfd = 0;
	else
		curfd = open(argv[2], O_RDONLY);

	/* open output file */
	if (!strncmp(argv[3], "-", 1))
		outfd = 1;
	else
		outfd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (read(prevfd, &prevbuf, sizeof(prevbuf)) != sizeof(prevbuf))
		perror("prev read");

	if (read(curfd, &curbuf, sizeof(curbuf)) != sizeof(curbuf))
		perror("cur read");

	for (i = 0; i < RAW_VERT_PIXELS; i++)
		for (j = 0; j < (RAW_HORIZ_PIXELS/2); j+=2) {
			int offset = (i * (RAW_HORIZ_PIXELS/2)) + j;
			int diff;

			diff = prevbuf[i][j] != curbuf[i][j] ||
				prevbuf[i][j+1] != curbuf[i][j+1];
			if (emit && (!diff || emitwords == 8)) {
				int jumpoff = outsize - 2 - emitwords * 2;

				/* fixup jump marker */
				outbuf[jumpoff] |= emitwords - 1;
				/* stop this run */
				emit = 0; emitwords = 0;
			}
			if (!emit && diff) {
				/* emit jump marker */
				outbuf[outsize++] |= 
					(offset & 0x3e00) >> 6;
				outbuf[outsize++] |=
					(offset & 0x01fe) >> 1;
				/* start emitting data words */
				emit = 1;
			}
			if (emit) {
				/* emit video data */
				outbuf[outsize++] = curbuf[i][j];
				outbuf[outsize++] = curbuf[i][j+1];
				/* bump word count */
				emitwords++;
			}
		}

	/* fixup last jump marker, if appropriate */
	if (emit) {
		int jumpoff = outsize - 2 - emitwords * 2;

		/* fixup jump marker */
		outbuf[jumpoff] |= emitwords - 1;
	}

	/* Always have at least a frame delimeter... */
	outbuf[outsize++] = 0xff;
	outbuf[outsize++] = 0x00;

	if (write(outfd, &outbuf, outsize) != outsize)
		perror("pixel write");

	close(prevfd);
	close(curfd);
	close(outfd);

	return 0;
}
