#ifndef CHUNK_H
#define CHUNK_H

#include <SDL/SDL_types.h>
#include "moag_net.h"

namespace moag
{

const int SEND_ALL = 0xFFFFFFFF;
const int FLUSH_QUEUE = 1;

void ChunkEnqueue8(Uint8 ch);
void ChunkEnqueue16(Uint16 ch);
void ChunkEnqueue32(Uint32 ch);
Uint8 ChunkDequeue8();
Uint16 ChunkDequeue16();
Uint32 ChunkDequeue32();
Uint32 IncomingChunkLength();
Uint32 OutgoingChunkLength();
int SendChunk(moag::Connection con, Uint32 bytes, int flush);
int ReceiveChunk(moag::Connection con, Uint32 bytes);

}

#endif

