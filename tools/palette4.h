#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <stdint.h>

struct rgb {
	uint8_t r, g, b;
};

struct rgb palette[] = {
#if COLORSET == 0
	{ 0, 255, 0, },
	{ 255, 255, 0, },
	{ 0, 0, 255, },
	{ 255, 0, 0, },
#elif COLORSET == 1
	{ 255, 255, 255, },
	{ 0, 255, 170, },
	{ 170, 85, 255, },
	{ 255, 85, 0, },
#elif COLORSET == 2
	{ 0, 0, 0, },
	{ 0, 85, 170, },
	{ 170, 85, 0, },
	{ 255, 255, 255, },
#else
#error "Unknown COLORSET value!"
#endif
};

#define PALETTE_SIZE	(sizeof(palette) / sizeof(palette[0]))

#endif /* _PALETTE_H_ */
