#include <stdio.h>
#include <math.h>

#include "palette.h"

#define MAXR	64
#define MAXG	64
#define MAXB	64

#define RGB(r, g, b)	((r * MAXG * MAXB) + (g * MAXB) + b)

/*
 * color_table is indexed by compacted r,g,b value and (once
 * initialized) returns closest known cmp256 value.
 */
uint8_t color_table[MAXR * MAXG * MAXB];

float rgb_distance(struct rgb c1, struct rgb c2)
{
	int rd, gd, bd;

	rd = c1.r - c2.r;
	gd = c1.g - c2.g;
	bd = c1.b - c2.b;

	return sqrtf((rd * rd) + (gd * gd) + (bd * bd));
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

				closest_distance = rgb_distance(cur,
							palette[0]);
				for (cmp = 1; cmp < PALETTE_SIZE; cmp++) {
					cur_distance = rgb_distance(cur,
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

void gen_header()
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
	printf("#define RGB(r, g, b)\t((((r * MAXR) / 256) * MAXG * MAXB) + \\\n");
	printf("\t\t\t (((g * MAXG) / 256) * MAXB) + \\\n");
	printf("\t\t\t  ((b * MAXB) / 256))\n");
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

int main(int argc, char *argv[])
{
	init_colors();
	/*
	 * show_matches();
	 */
	gen_header();

	return 0;
}
