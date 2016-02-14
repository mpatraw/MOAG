/*
 *
 *
 */
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

using buffer = std::vector<uint8_t>;

/* run length encoding
 * format: [byte data] [byte repetitions-1] repeat
 */
buffer rlencode(const buffer &src) {
    size_t pos = 0;
    size_t start = 0;
    uint8_t c = 0;
    buffer buf(src.size() * 2 + 1);

    for (size_t i = 0; i <= src.size(); ++i) { 
        if (i == src.size() || src[i] != c) {
            if (i > start) {
                while (i - start > 256) {
                    buf[pos++] = c;
                    buf[pos++] = 255;
                    start += 256;
                }
                buf[pos++] = c;
                buf[pos++] = (uint8_t)(i - start - 1);
            }
            if (i != src.size()) {
                c = src[i];
                start = i;
            }
        }
    }
    buf.resize(pos);
    return buf;
}

/* run length decoding
 */
buffer rldecode(const buffer &src) {
    buffer buf;
    // Some random buffer value to start off.
    buf.reserve(512);

    for (size_t i = 0; i < src.size(); i += 2) {
        uint8_t c = src[i];
        size_t n = (size_t)src[i + 1];
        for (size_t j = 0; j <= n; ++j) {
            buf.push_back(c);
        }
    }

    return buf;
}

