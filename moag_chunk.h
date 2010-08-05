
#ifndef MOAG_CHUNK_H
#define MOAG_CHUNK_H

#include <SDL/SDL_types.h>

#define MOAG_SEND_ALL 0xFFFFFFFF
#define MOAG_FLUSH_QUEUE 1

void MOAG_ChunkEnqueue8(Uint8 ch);
void MOAG_ChunkEnqueue16(Uint16 ch);
void MOAG_ChunkEnqueue32(Uint32 ch);
Uint8 MOAG_ChunkDequeue8(void);
Uint16 MOAG_ChunkDequeue16(void);
Uint32 MOAG_ChunkDequeue32(void);
Uint32 MOAG_IncomingChunkLength(void);
Uint32 MOAG_OutgoingChunkLength(void);
int MOAG_SendChunk(void *con, Uint32 bytes, int flush);
int MOAG_ReceiveChunk(void *con, Uint32 bytes);

#endif /* MOAG_CHUNK_H */

