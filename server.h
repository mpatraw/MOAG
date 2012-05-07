
#ifndef SERVER_H
#define SERVER_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <enet/enet.h>

#include "common.h"

#define MAX_BULLETS         256
#define GRAVITY             0.1
#define BOUNCER_BOUNCES     11
#define TUNNELER_TUNNELINGS 20
#define RESPAWN_TIME        40
#define LADDER_TIME         60
#define LADDER_LENGTH       64

#endif
