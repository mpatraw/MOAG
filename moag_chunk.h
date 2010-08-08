#ifndef MOAG_CHUNK_H
#define MOAG_CHUNK_H

#include <SDL/SDL_types.h>
#include "moag_net.h"

#define MOAG_SEND_ALL 0xFFFFFFFF
#define MOAG_FLUSH_QUEUE 1

void MOAG_ChunkEnqueue8(Uint8 ch);
void MOAG_ChunkEnqueue16(Uint16 ch);
void MOAG_ChunkEnqueue32(Uint32 ch);
Uint8 MOAG_ChunkDequeue8();
Uint16 MOAG_ChunkDequeue16();
Uint32 MOAG_ChunkDequeue32();
Uint32 MOAG_IncomingChunkLength();
Uint32 MOAG_OutgoingChunkLength();
int MOAG_SendChunk(MOAG_Connection con, Uint32 bytes, int flush);
int MOAG_ReceiveChunk(MOAG_Connection con, Uint32 bytes);

#endif

