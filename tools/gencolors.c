/*
 * Copyright (c) 2009-2011, John W. Linville <linville@tuxdriver.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <math.h>

#if defined(COLORS)
#if COLORS == 16
#include "palette16.h"
#define MAXR	8
#define MAXG	8
#define MAXB	8
#elif COLORS == 256
#include "palette256.h"
#define MAXR	64
#define MAXG	64
#define MAXB	64
#elif COLORS == 4
#include "palette4.h"
#define MAXR	4
#define MAXG	4
#define MAXB	4
#else
#error "Unknown value for COLORS!"
#endif
#endif

#define RGB(r, g, b)	((r * MAXG * MAXB) + (g * MAXB) + b)

/*
 * color_table is indexed by compacted r,g,b value and (once
 * initialized) returns closest known cmp256 value.
 */
uint8_t color_table[MAXR * MAXG * MAXB];

/*
 * distance_table is indexed by two palette colors and
 * returns the distance between them in the YIQ cube.
 *
 * (NOTE: uint16_t is OK due to fractional multipliers in
 *        conversion to YIQ colorspace.)
 */
uint16_t distance_table[COLORS][COLORS];

struct yiq {
        float y, i, q;
};

float yiq_distance(struct rgb c1, struct rgb c2)
{
	struct yiq y1, y2;
	float yd, id, qd;

	y1.y = 0.299000*c1.r + 0.587000*c1.g + 0.114000*c1.b;
	y1.i = 0.595716*c1.r - 0.274453*c1.g - 0.321263*c1.b;
	y1.q = 0.211456*c1.r - 0.522591*c1.g + 0.311135*c1.b;

	y2.y = 0.299000*c2.r + 0.587000*c2.g + 0.114000*c2.b;
	y2.i = 0.595716*c2.r - 0.274453*c2.g - 0.321263*c2.b;
	y2.q = 0.211456*c2.r - 0.522591*c2.g + 0.311135*c2.b;

	yd = y1.y - y2.y;
	id = y1.i - y2.i;
	qd = y1.q - y2.q;

	/* return sum of squares to better differentiate colors */
	return (yd * yd) + (id * id) + (qd * qd);
}

void init_colors()
{
	int r, g, b, cmp;
	struct rgb cur;

	/*
	 * initialize color_table
	 */
	for (r = 0; r < MAXR; r++) {
		cur.r = r * 256 / MAXR;
		for (g = 0; g < MAXG; g++) {
			cur.g = g * 256 / MAXG;
			for (b = 0; b < MAXB; b++) {
				float cur_distance, closest_distance;
				int closest = 0;

				cur.b = b * 256 / MAXB;

				closest_distance = yiq_distance(cur,
							palette[0]);
				for (cmp = 1; cmp < PALETTE_SIZE; cmp++) {
					cur_distance = yiq_distance(cur,
								palette[cmp]);
					if (cur_distance < closest_distance) {
						closest_distance = cur_distance;
						closest = cmp;
					}
				}

				color_table[RGB(r,g,b)] = closest;
			}
		}
	}
}

void show_matches()
{
	int i;

	for (i = 0; i < (MAXR * MAXG * MAXB); i++) {
		printf("RGB %3d, %3d, %3d => COLOR %3d\n",
			(i & (MAXR - 1) * MAXG * MAXB) / (MAXG * MAXB),
			(i & (MAXG - 1) * MAXB) / MAXB,
			 i & (MAXB - 1),
			color_table[i]);
	}
}

void init_distance()
{
	int i, j;

	for (i = 0; i < COLORS; i++)
		for (j = 0; j < COLORS; j++) {
			distance_table[i][j] =
				(uint16_t)(yiq_distance(palette[i],
							palette[j]) + 0.5);
		}
}

void gen_colors()
{
	int i;

	printf("#ifndef _COLORS_H_\n");
	printf("#define _COLORS_H_\n");
	printf("\n");
	printf("#include <stdint.h>\n");
	printf("\n");
	printf("#define MAXR\t%d\n", MAXR);
	printf("#define MAXG\t%d\n", MAXG);
	printf("#define MAXB\t%d\n", MAXB);
	printf("\n");
	printf("#define MASKR\t(256 - (256 / MAXR))\n");
	printf("#define MASKG\t(256 - (256 / MAXG))\n");
	printf("#define MASKB\t(256 - (256 / MAXB))\n");
	printf("\n");
	printf("#define MIN(a, b)\t(a < b ? a : b)\n");
	printf("\n");
	printf("#define RGB(r, g, b)\t(((((MIN(255, (r + (256 / MAXR / 2))) & MASKR) \\\n");
	printf("\t\t\t\t* MAXR) / 256) * MAXG * MAXB) + \\\n");
	printf("\t\t\t ((((MIN(255, (g + (256 / MAXR / 2))) & MASKG) \\\n");
	printf("\t\t\t\t* MAXG) / 256) * MAXB) + \\\n");
	printf("\t\t\t  (((MIN(255, (b + (256 / MAXR / 2))) & MASKB) \\\n");
	printf("\t\t\t\t* MAXB) / 256))\n");
	printf("\n");
	printf("uint8_t color[] = {\n");

	for (i = 0; i < (MAXR * MAXG * MAXB); i++) {
		if (i % 8 == 0)
			printf("\t0x%02x,", color_table[i]);
		else
			printf(" 0x%02x,", color_table[i]);
		if (i % 8 == 7)
			printf("\n");
	}

	printf("};\n");
	printf("\n");
	printf("#endif /* _COLORS_H_ */\n");
}

void gen_distance()
{
	int i, j;

	printf("#ifndef _DISTANCE_H_\n");
	printf("#define _DISTANCE_H_\n");
	printf("\n");
	printf("#include <stdint.h>\n");
	printf("\n");
	printf("uint16_t distance[%d][%d] = {\n", COLORS, COLORS);


	for (i = 0; i < COLORS; i++) {
		printf("\t{\n");
		for (j = 0; j < COLORS; j++) {
			if (j % 8 == 0)
				printf("\t\t0x%04x,", distance_table[i][j]);
			else
				printf(" 0x%04x,", distance_table[i][j]);
			if (j % 8 == 7)
				printf("\n");
		}
		printf("\t},\n");
	}

	printf("};\n");
	printf("\n");
	printf("#endif /* _DISTANCE_H_ */\n");
}

int main(int argc, char *argv[])
{
	init_colors();
	/*
	 * show_matches();
	 */
	init_distance();

	/* Could do a better job of checking args... */
	if (argc < 2)
		gen_colors();
	else 
		gen_distance();

	return 0;
}
