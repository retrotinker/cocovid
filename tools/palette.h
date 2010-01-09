#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <stdint.h>

struct rgb {
	uint8_t r, g, b;
};

struct rgb palette[] = {
	{ 0, 0, 0 },
	{ 0, 0, 160 },
	{ 0, 160, 0 },
	{ 0, 160, 160 },
	{ 160, 0, 0, },
	{ 160, 0, 160 },
	{ 160, 160, 0 },
	{ 160, 160, 160 },
	{ 95, 95, 95, },
	{ 95, 95, 255 },
	{ 95, 255, 95 },
	{ 95, 255, 255 },
	{ 255, 95, 95, },
	{ 255, 95, 255 },
	{ 255, 255, 95 },
	{ 255, 255, 255 },
};

#define PALETTE_SIZE	(sizeof(palette) / sizeof (palette[0]))

#endif /* _PALETTE_H_ */
