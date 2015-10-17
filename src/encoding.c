/*
 *
 *
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* run length encoding
 * format: [byte data] [byte repetitions-1] repeat
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen) {
	size_t pos = 0;
	size_t start = 0;
	uint8_t c = 0;
	uint8_t *buf = malloc(len*2+1);
	if (!buf) {
		return NULL;
	}

	for (size_t i = 0; i <= len; ++i) {
		if (i == len || src[i] != c) {
			if (i > start) {
				while (i - start > 256) {
					buf[pos++] = c;
					buf[pos++] = 255;
					start += 256;
				}
				buf[pos++] = c;
				buf[pos++] = (uint8_t)(i-start-1);
			}
			if (i != len) {
				c = src[i];
				start = i;
			}
		}
	}

	buf = realloc(buf, pos);
	*outlen = pos;
	return buf;
}

/* run length decoding
 */
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen) {
	size_t bufsize = 512;
	size_t pos = 0;
	uint8_t *buf = malloc(bufsize);

	if (!buf) {
		return NULL;
	}

	for (size_t i = 0; i < len; i += 2) {
		uint8_t c = src[i];
		size_t n = (size_t)src[i+1];
		for (size_t j = 0; j <= n; ++j) {
			if (pos >= bufsize) {
				bufsize = (int)(bufsize * 1.3);
				buf = realloc(buf, bufsize);
				if (!buf) {
					return NULL;
				}
			}
			buf[pos++] = c;
		}
	}

	buf = realloc(buf, pos);
	*outlen = pos;

	return buf;
}
