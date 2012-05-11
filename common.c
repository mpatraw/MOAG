
#include <zlib.h>

#include "common.h"

int zip(char *in_data, size_t in_size, char **out_data, size_t *out_size)
{
    int ret;
    z_stream strm;

    *out_data = malloc(in_size + 4);

    /*copy the original size to our output buffer*/
    memcpy(*out_data, &in_size, 4);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit2(&strm, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return ret;
    }

    strm.avail_in = in_size;
    strm.next_in = (uint8_t*)in_data;
    strm.avail_out = in_size;
    strm.next_out = (uint8_t*)*out_data+4;

    ret = deflate(&strm, Z_FINISH);

    if (ret == Z_STREAM_ERROR)
        return Z_STREAM_ERROR;
    if (strm.avail_in != 0)
        return Z_ERRNO;
    if (ret != Z_STREAM_END)
        return Z_STREAM_END;
    /*if you get the error above you probably need to allow more space for compression*/

    *out_size = (in_size) - strm.avail_out + 4; /*+4 for original added size*/

    (void)deflateEnd(&strm);
    return Z_OK;
}

int unzip(char *in_data, size_t in_size, char **out_data, size_t *out_size)
{
    int ret;
    unsigned expected_dest_size;
    z_stream strm;

    /*grab the expected size off the front of the buffer*/
    memcpy(&expected_dest_size, in_data, 4);
    in_data += 4;
    in_size -= 4;

    *out_data = malloc(expected_dest_size);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = inflateInit2(&strm, -15);
    if (ret != Z_OK) {
        return ret;
    }

    strm.avail_in = in_size;
    strm.next_in = (uint8_t*)in_data;
    strm.avail_out = expected_dest_size;
    strm.next_out = (uint8_t*)*out_data;

    ret = inflate(&strm, Z_FINISH);
    if (ret == Z_STREAM_ERROR)
        return Z_STREAM_ERROR;
    if ( ret != Z_STREAM_END )
        return Z_DATA_ERROR;
    if ( strm.avail_in != 0 )
        return Z_ERRNO;
    if ( strm.avail_out != 0 )
        return Z_ERRNO;

    *out_size = expected_dest_size;

    (void)inflateEnd(&strm);
    return Z_OK;
}

