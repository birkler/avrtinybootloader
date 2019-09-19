#include <stdint.h>
#include <stdio.h>


void lfsr16() {
	uint16_t lfsr = 0xACE1u;
	uint32_t period = 0;
	do {
		unsigned char lsb = lfsr & 1;  /* Get lsb (i.e., the output bit). */
		lfsr >>= 1;               /* Shift register */
		if (lsb == 1)             /* Only apply toggle mask if output bit is 1. */
		lfsr ^= 0xB400u;        /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
		++period;
	} while (lfsr != 0xACE1u);
	printf("lfsr16 period:%d\n",period);
}


uint8_t lfsr8a() {
	static uint8_t lfsr = 0x01;
	unsigned char lsb = lfsr & 1;  /* Get lsb (i.e., the output bit). */
	lfsr >>= 1;               /* Shift register */
	if (lsb == 1)             /* Only apply toggle mask if output bit is 1. */  //[1,2,3,7]
		lfsr ^= (1<<7 | 1<<3 | 1<<2 | 1<<1);        /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
	return lfsr;
}

uint8_t lfsr8() {
	static uint8_t lfsr = 0x01;
	unsigned char lsb = lfsr & 1;  /* Get lsb (i.e., the output bit). */
	lfsr >>= 1;               /* Shift register */
	if (lsb == 1)             /* Only apply toggle mask if output bit is 1. */  //[1,2,3,7]
		lfsr ^= (1<<3 | 1<<4 | 1<<2 | 1<<1);        /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
	return lfsr;
}


uint8_t lfsr8_test() {
	uint32_t period = 0;
	uint8_t lfsr;
	uint8_t start = lfsr8();
	do {
		lfsr = lfsr8();
		printf("%02x,",lfsr);
		++period;
	} while (lfsr != start);
	printf("lfsr8 period:%d\n",period);
}


int main(int argc, char* argv[])
{
	lfsr16();
	lfsr8_test();
}


