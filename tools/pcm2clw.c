#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unsigned char pcmbuf[1024];
unsigned char outbuf[512];

unsigned char clawtbl[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 

	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,

	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,

	0x04, 0x04, 0x04, 0x04,

	0x05, 0x05,

	0x06,

	0x07,

	0x08,

	0x09,

	0x0a, 0x0a,

	0x0b, 0x0b, 0x0b, 0x0b,

	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,

	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,

	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,

	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int rc, val;
	int pcmidx = 0, outidx = 0;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while (rc = read(infd, &pcmbuf, sizeof(pcmbuf)) == sizeof(pcmbuf)) {
		while (pcmidx < sizeof(pcmbuf)) {
			val = clawtbl[pcmbuf[pcmidx++]] << 4;
			val |= clawtbl[pcmbuf[pcmidx++]];
			outbuf[outidx++] = val;
		}
		if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
			perror("clw write");
		pcmidx = 0;
		outidx = 0;
	}

	if (rc < 0)
		perror("pcm read");
	else {
		while (pcmidx < rc) {
			if (pcmidx & 1) {
				val |= clawtbl[pcmbuf[pcmidx++]];
				outbuf[outidx++] = val;
			} else
				val = clawtbl[pcmbuf[pcmidx++]] << 4;
		}
		if (pcmidx & 1)
			outbuf[outidx++] = val;
		if (write(outfd, outbuf, outidx) != outidx)
				perror("clw write");
	}

	close(infd);
	close(outfd);

	return 0;
}
