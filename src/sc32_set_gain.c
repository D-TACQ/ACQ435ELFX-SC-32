/* sc32_set_gain ...
 *
 *
 * SITE=[1] sc32_set_gain STROBE_MASK G21ww G21xx G21yy G21zz
 *
 *
 * sc32_set_gain 1
 *
 * SITE: 1,3,5
 *
 *
 * sc_set_gain	0x01	G2113	G2109	G2105
 * sc_set_gain	0x02	G2114	G2110	G2106
 * sc_set_gain	0x04	G2115	G2111	G2107
 * sc_set_gain	0x08	G2116	G2112	G2108
 * sc_set_gain	0x10	G2129	G2125	G2121
 * sc_set_gain	0x20	G2130	G2126	G2122
 * sc_set_gain	0x40	G2131	G2127	G2123
 * sc_set_gain	0x80	G2132	G2128	G2124
 *
 */


#include <errno.h>
#include <gpiod.h>
#include <getopt.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include <stdlib.h>



int site = 1;
char chip20[4];
char chip21[4];

int verbose = 0;

unsigned OFFSETS[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
int ONES[]         = { 1, 1, 1, 1, 1, 1, 1, 1 };

int get_knob(int site, int chip){
	char fname[80];
	FILE *fp;
	sprintf(fname, "/dev/gpio/ELFX_SC/%d/chip2%d", site, chip);
	fp = fopen(fname, "r");
	if (fp){
		int value;
		if (fscanf(fp, "%d", &value) == 1){
			fclose(fp);
			return value;
		}
	}
	perror(fname);
	exit(1);
}

void set_bits(const char* device, unsigned *offsets, int *values, int nbits)
{
	int rv;

	if (verbose){
		int ib;
		printf("set_bits %s %d:");
		for (ib = 0; ib < nbits; ++ib){
			printf("%d=%d ", offsets[ib], values[ib]);
		}
		printf("\n");
	}
	rv = gpiod_ctxless_set_value_multiple(
			device, offsets, values, nbits, 0, "gpioset",  NULL, 0);
	if (rv < 0){
		perror("error setting the GPIO line values");
		exit(1);
	}
}

int main(int argc, char* argv[])
{

	int ii;
	int maskbits[8];
	int values[16];
	unsigned iv = 16;

	if (getenv("SITE")){
		site = atoi(getenv("SITE"));
	}
	if (getenv("SC32_VERBOSE")){
		verbose = atoi(getenv("SC32_VERBOSE"));
	}
	sprintf(chip20, "%d", get_knob(site, 0));
	sprintf(chip21, "%d", get_knob(site, 1));

	for (ii = 1; ii < argc && ii <= 1+1+4; ++ii){
		unsigned arg = strtoul(argv[ii], 0, 0);
		if (ii==1){
			unsigned cursor = 8;
			unsigned mask = arg&0xff;
			unsigned im = 0;
			for ( ; cursor--; im++ ){
				maskbits[im] = !(mask&(1<<cursor));
			}
		}else{
			values[--iv] = arg&0x8;
			values[--iv] = arg&0x4;
			values[--iv] = arg&0x2;
			values[--iv] = arg&0x1;
		}
	}

	if (ii == 1){
		printf("query todo\n");
		return 0;
	}
	set_bits(chip20, OFFSETS, values, 16);
	set_bits(chip21, OFFSETS, maskbits, 8);
	set_bits(chip21, OFFSETS, ONES, 8);
	return 0;
}
