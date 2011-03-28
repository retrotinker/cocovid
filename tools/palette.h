#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <stdint.h>

struct rgb {
	uint8_t r, g, b;
};

struct rgb palette[] = {
	{ 0, 0, 0, },
	{ 0, 0, 255, },
	{ 0, 255, 0, },
	{ 0, 255, 255, },
	{ 255, 0, 0, },
	{ 255, 0, 255, },
	{ 255, 255, 0, },
	{ 255, 255, 255, },
	{ 85, 85, 85, },
	{ 85, 85, 170, },
	{ 85, 170, 85, },
	{ 85, 170, 170, },
	{ 170, 85, 85, },
	{ 170, 85, 170, },
	{ 170, 170, 85, },
	{ 170, 170, 170, },
};

#define PALETTE_SIZE	(sizeof(palette) / sizeof (palette[0]))

#endif /* _PALETTE_H_ */
