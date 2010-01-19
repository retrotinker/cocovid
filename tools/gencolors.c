#include <stdio.h>
#include <math.h>

#include "palette.h"

#define MAXR	8
#define MAXG	8
#define MAXB	8

#define RGB(r, g, b)	((r * MAXG * MAXB) + (g * MAXB) + b)

/*
 * color_table is indexed by compacted r,g,b value and (once
 * initialized) returns closest known cmp256 value.
 */
uint8_t color_table[MAXR * MAXG * MAXB];

/*
 * distance_table is indexed by two palette colors and
 * returns the distance between them in the RGB cube.
 */
uint16_t distance_table[16][16];

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

void init_distance()
{
	int i, j;

	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++) {
			distance_table[i][j] =
				(uint16_t)(rgb_distance(palette[i],
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
	printf("uint16_t distance[16][16] = {\n");


	for (i = 0; i < 16; i++) {
		printf("\t{\n");
		for (j = 0; j < 16; j++) {
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
