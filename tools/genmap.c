#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "palette.h"

#define PPM_HEADER	"P6\n16 1\n255\n"
#define PPM_HEADER_SIZE	12

void usage(char *prg)
{
	printf("Usage: %s\n", prg);
}

int main(int argc, char *argv[])
{
	int i;

	if (argc > 1) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* output PPM header */
	if (write(1, PPM_HEADER, PPM_HEADER_SIZE) != PPM_HEADER_SIZE)
		perror("PPM header write");

	/* output palette data */
	for (i = 0; i < PALETTE_SIZE; i++) {
		if (write(1, &palette[i].r, 1) != 1)
			perror("pixel write: r");
		if (write(1, &palette[i].g, 1) != 1)
			perror("pixel write: g");
		if (write(1, &palette[i].b, 1) != 1)
			perror("pixel write: b");
	}

	return 0;
}
