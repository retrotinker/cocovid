#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unsigned char wavbuf[1024];
unsigned char outbuf[1024];

void usage(char *prg)
{
	printf("Usage: %s infile outfile [factor]\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int rc;
	int wavidx = 0, outidx = 0;
	int factor = 1;
	char hdbuf[44];

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((argc > 3) && (sscanf(argv[3], "%d", &factor) < 0)) {
		perror("sscanf factor");
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (read(infd, hdbuf, sizeof(hdbuf)) != sizeof(hdbuf))
		perror("head read");

	while ((rc = read(infd, &wavbuf, sizeof(wavbuf))) == sizeof(wavbuf)) {
		while (wavidx < sizeof(wavbuf)) {
			outbuf[outidx++] = wavbuf[wavidx];
			wavidx += factor;
		}
		if (outidx == sizeof(outbuf)) {
			if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
				perror("pcm write");
			outidx = 0;
		} else if (outidx > sizeof(outbuf)) {
			printf("outidx too big!\n");
			exit(EXIT_FAILURE);
		}
		wavidx = 0;
	}

	if (rc < 0)
		perror("wav read");
	else {
		while (wavidx < rc) {
			outbuf[outidx++] = wavbuf[wavidx];
			wavidx += factor;
		}
		if (write(outfd, outbuf, outidx) != outidx)
				perror("pcm write");
		else if (outidx > sizeof(outbuf)) {
			printf("outidx too big!\n");
			exit(EXIT_FAILURE);
		}
	}

	close(infd);
	close(outfd);

	return 0;
}
