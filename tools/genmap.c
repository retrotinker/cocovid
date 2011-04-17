#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(COLORS)
#if COLORS == 16
#include "palette16.h"
#define PPM_HEADER	"P6\n16 1\n255\n"
#define PPM_HEADER_SIZE	12
#elif COLORS == 256
#include "palette256.h"
#define PPM_HEADER	"P6\n256 1\n255\n"
#define PPM_HEADER_SIZE	13
#elif COLORS == 4
#include "palette4.h"
#define PPM_HEADER	"P6\n4 1\n255\n"
#define PPM_HEADER_SIZE	11
#else
#error "Unknown value for COLORS!"
#endif
#endif

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
