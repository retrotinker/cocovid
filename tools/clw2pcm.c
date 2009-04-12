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
#ifdef SIMULATE_COCO_DAC /* only msb 6-bit, simulates CoCo on-board DAC */
	0x00, 0x40, 0x60, 0x70, 0x78, 0x7c, 0x7c, 0x7c,
	0x80, 0x80, 0x80, 0x84, 0x8c, 0x9c, 0xbc, 0xfc,
#else /* 8-bit output */
	0x00, 0x40, 0x60, 0x70, 0x78, 0x7c, 0x7e, 0x7f,
	0x80, 0x81, 0x83, 0x87, 0x8f, 0x9f, 0xbf, 0xff,
#endif
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
