
/* random number generation
 */
#ifndef XOR128_H
#define XOR128_H

#include <stdint.h>

enum {XOR128_K = 4};

struct rng_state
{
	uint32_t q[XOR128_K];
};

void rng_seed(struct rng_state *st, uint32_t seed);
uint32_t rng_u32(struct rng_state *st);
double rng_unit(struct rng_state *st);
double rng_under(struct rng_state *st, int32_t max);
double rng_between(struct rng_state *st, int32_t min, int32_t max);
int32_t rng_range(struct rng_state *st, int32_t min, int32_t max);

#endif
