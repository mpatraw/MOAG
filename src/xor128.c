/* Xor128 RNG
 * Fast speed. Low memory. Period = 2^128 - 1. 
 */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "xor128.h"

void rng_seed(struct rng_state *st, uint32_t seed)
{
	int i;

	srand(seed);
	for (i = 0; i < XOR128_K; ++i)
		st->q[i] = rand();
}

uint32_t rng_u32(struct rng_state *st)
{
	uint32_t t;
	t = (st->q[0] ^ (st->q[0] << 11));
	st->q[0] = st->q[1];
	st->q[1] = st->q[2];
	st->q[2] = st->q[3];
	return st->q[3] = st->q[3] ^ (st->q[3] >> 19) ^ (t ^ (t >> 8));
}

double rng_unit(struct rng_state *st)
{
	return rng_u32(st) * 2.3283064365386963e-10;
}

double rng_under(struct rng_state *st, int32_t max)
{
	return rng_unit(st) * max;
}

double rng_between(struct rng_state *st, int32_t min, int32_t max)
{
	return rng_under(st, max - min) + min;
}

int32_t rng_range(struct rng_state *st, int32_t min, int32_t max)
{
	return floor(rng_between(st, min, max + 1));
}
