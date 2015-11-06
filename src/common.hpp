
#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <enet/enet.h>

#include "moag.hpp"

#define SQ(x)           ((x) * (x))

#ifndef M_PI
#   define M_PI         3.14159
#endif

#define DEG2RAD(deg)    ((deg) * (M_PI / 180))
#define RAD2DEG(rad)    ((rad) * (180 / M_PI))

/******************************************************************************\
Shared structures.
\******************************************************************************/

/* Chunk types. */
enum {
    /******************
     * Client -> Server
     */

    /* RELIABLE
     * 1: INPUT_CHUNK
     * 1: *_*_CHUNK
     * 2: Milliseconds held.
     */
    INPUT_CHUNK = 2,

    /* RELIABLE
     * 1: CLIENT_MSG_CHUNK
     * length: characters
     */
    CLIENT_MSG_CHUNK,

    /******************
     * Server -> Client
     */

    /* RELIABLE
     * 1: LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * X: zipped land (columns first)
     */
    LAND_CHUNK,
    /* RELIABLE
     * 1: PACKED_LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * 4: packed-size
     * X: RLE-compressed data
     */
    PACKED_LAND_CHUNK,
    /* VARIES
     * 1: TANK_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     *  1: angle (0 to 180), 0 is left, 180 is right
     */
    TANK_CHUNK,
    /* VARIES
     * 1: BULLET_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    BULLET_CHUNK,
    /* VARIES
     * 1: CREATE_CHUNK
     * 1: SPAWN/KILL/MOVE
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    CRATE_CHUNK,
    /* RELIABLE
     * 1: SERVER_MSG_CHUNK
     * 1: id
     * 1: CHAT/NAME_CHANGE/SERVER_NOTICE
     * length: characters
     */
    SERVER_MSG_CHUNK,
};

/* Input types.
 * KFIRE_RELEASED is special. It has extra info on the time spent charging up.
 */
enum {
    KLEFT_PRESSED, KLEFT_RELEASED,
    KRIGHT_PRESSED, KRIGHT_RELEASED,
    KUP_PRESSED, KUP_RELEASED,
    KDOWN_PRESSED, KDOWN_RELEASED,
    KFIRE_PRESSED, KFIRE_RELEASED,
};

enum {
    /* RELIABLE */
    SPAWN,
    /* RELIABLE */
    KILL,
    /* UNRELIABLE */
    MOVE,
};

/* SERVER_MSG_CHUNK commands */
enum {
    CHAT,
    SERVER_NOTICE,
    NAME_CHANGE,
};


#endif
