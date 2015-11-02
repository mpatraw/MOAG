/*
 *
 */
#ifndef MOAG_H
#define MOAG_H

#include <stddef.h>
#include <stdint.h>

#include "xor128.h"

/* logging macros
 */
#define ERR(...) \
do { fprintf(stderr, "! "); fprintf(stderr, __VA_ARGS__); } while (0)
#define DIE(...) do { ERR(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

/* encoding/decoding
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen);
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen);

#endif
