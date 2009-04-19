#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unsigned char clwbuf[512];
unsigned char outbuf[1024];

unsigned char pcmtbl[16] = {
	0x04, 0x18, 0x2c, 0x40, 0x54, 0x60, 0x6c, 0x78,
	0x84, 0x90, 0x9c, 0xa8, 0xbc, 0xd0, 0xe4, 0xf8,
};

void usage(char *prg)
{
	printf("Usage: %s infile outfile\n", prg);
}

int main(int argc, char *argv[])
{
	int infd, outfd;
	int rc, val;
	int clwidx = 0, outidx = 0;

	if (argc < 3) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* open input file */
	infd = open(argv[1], O_RDONLY);

	/* open output file */
	outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	while (rc = read(infd, &clwbuf, sizeof(clwbuf)) == sizeof(clwbuf)) {
		while (clwidx < sizeof(clwbuf)) {
			val = clwbuf[clwidx++];
			outbuf[outidx++] = pcmtbl[(val & 0xf0) >> 4];
			outbuf[outidx++] = pcmtbl[val & 0x0f];
		}
		if (write(outfd, outbuf, sizeof(outbuf)) != sizeof(outbuf))
			perror("clw write");
		clwidx = 0;
		outidx = 0;
	}

	if (rc < 0)
		perror("clw read");
	else {
		while (clwidx < rc) {
			val = clwbuf[clwidx++];
			outbuf[outidx++] = pcmtbl[(val & 0xf0) >> 4];
			outbuf[outidx++] = pcmtbl[val & 0x0f];
		}
		if (write(outfd, outbuf, outidx) != outidx)
				perror("clw write");
	}

	close(infd);
	close(outfd);

	return 0;
}