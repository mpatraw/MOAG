/*
 *
 */
#ifndef MOAG_H
#define MOAG_H

#include <stddef.h>
#include <stdint.h>

#include "config.hpp"

class land
{
public:
};

/* encoding/decoding
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen);
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen);

#endif
