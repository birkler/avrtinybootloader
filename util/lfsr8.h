
static uint8_t prng_lfsr8_255(void) {
	static uint8_t lfsr = 0x01;
	unsigned char lsb = lfsr & 1;  /* Get lsb (i.e., the output bit). */
	lfsr >>= 1;               /* Shift register */
	if (lsb == 1)             /* Only apply toggle mask if output bit is 1. */  //[1,2,3,7]
		lfsr ^= (1<<7 | 1<<3 | 1<<2 | 1<<1);        /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
	return lfsr;
}

static uint8_t prng_lfsr8_31(void) {
	static uint8_t lfsr = 0x01;
	unsigned char lsb = lfsr & 1;  /* Get lsb (i.e., the output bit). */
	lfsr >>= 1;               /* Shift register */
	if (lsb == 1)             /* Only apply toggle mask if output bit is 1. */  //[1,2,3,7]
		lfsr ^= (1<<3 | 1<<4 | 1<<2 | 1<<1);        /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
	return lfsr;
}


static uint8_t prng_lfsr1_1(void) {
	static uint8_t lfsr = 0x01;
	lfsr ^= 0x1;
	return lfsr;
}
