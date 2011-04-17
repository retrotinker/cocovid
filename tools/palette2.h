#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <stdint.h>

/* This is a dummy file.  Since the monochrome palette is so simple,
 * the utilities just manipulate the pixel values directly.  However,
 * the other palettes define struct rgb, and raw2ppm needs that structure.
 * So, define it here...
 */

struct rgb {
	uint8_t r, g, b;
};

#endif /* _PALETTE_H_ */
