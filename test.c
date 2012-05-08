
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void print_buf(unsigned char *buf, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        printf("%d ", buf[i]);
    }
    printf("\n");
}

int main(void)
{
    unsigned char buf[256] = {0};
    size_t pos = 0;

    print_buf(buf, pos);

    build_land_chunk(buf, &pos);

    print_buf(buf, pos);

    pos = 0;

    printf("%c\n", read8(buf, &pos));
    printf("%c\n", read8(buf, &pos));
    printf("%c\n", read8(buf, &pos));
    printf("%c\n", read8(buf, &pos));
    printf("%d\n", read16(buf, &pos));

    return EXIT_SUCCESS;
}
