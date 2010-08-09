
#include "moag_chunk.h"
#include "moag_net.h"
#include <assert.h>

namespace moag
{

// Implemented as a resizing circular buffer.
Uint8 *_incoming = NULL;
Uint32 _inFront = 0;
Uint32 _inLen = 0;
Uint32 _inAlloc = 0;

// Implemented as a simple array.
Uint8 *_outgoing = NULL;
Uint32 _outLen = 0;
Uint32 _outAlloc = 0;

void ChunkEnqueue8(Uint8 ch) {
    if (!_outgoing) {
        _outAlloc = 4;
        _outgoing = (Uint8 *)malloc(_outAlloc);
    }
    
    if (_outLen + 1 > _outAlloc) {
        _outAlloc *= 2;
        _outgoing = (Uint8 *)realloc(_outgoing, _outAlloc);
    }
    
    _outgoing[_outLen] = ch;
    _outLen += 1;
}

void ChunkEnqueue16(Uint16 ch) {
    if (!_outgoing) {
        _outAlloc = 4;
        _outgoing = (Uint8 *)malloc(_outAlloc);
    }
    
    if (_outLen + 2 > _outAlloc) {
        _outAlloc *= 2;
        _outgoing = (Uint8 *)realloc(_outgoing, _outAlloc);
    }
    
    Pack16(ch, &_outgoing[_outLen]);
    _outLen += 2;
}

void ChunkEnqueue32(Uint32 ch) {
    if (!_outgoing) {
        _outAlloc = 4;
        _outgoing = (Uint8 *)malloc(_outAlloc);
    }
    
    if (_outLen + 4 > _outAlloc) {
        _outAlloc *= 2;
        _outgoing = (Uint8 *)realloc(_outgoing, _outAlloc);
    }
    
    Pack32(ch, &_outgoing[_outLen]);
    _outLen += 4;
}


Uint8 ChunkDequeue8() {
    Uint8 rv;
    
    assert(1 <= _inLen && "underflow");
    rv = _incoming[_inFront];
    _inFront += 1;
    _inLen -= 1;

    if (_inLen == 0) {
        free(_incoming);
        _incoming = NULL;
        _inFront = 0;
        _inAlloc = 0;
    }
    
    return rv;
}

Uint16 ChunkDequeue16() {
    Uint16 rv;
    
    assert(2 <= _inLen && "underflow");
    rv = Unpack16(&_incoming[_inFront]);
    _inFront += 2;
    _inLen -= 2;
    
    if (_inLen == 0) {
        free(_incoming);
        _incoming = NULL;
        _inFront = 0;
        _inAlloc = 0;
    }
    
    return rv;
}

Uint32 ChunkDequeue32() {
    Uint32 rv;
    
    assert(4 <= _inLen && "underflow");
    rv = Unpack32(&_incoming[_inFront]);
    _inFront += 4;
    _inLen -= 4;
    
    if (_inLen == 0) {
        free(_incoming);
        _incoming = NULL;
        _inFront = 0;
        _inAlloc = 0;
    }
    
    return rv;
}

Uint32 IncomingChunkLength() {
    return _inLen;
}

Uint32 OutgoingChunkLength() {
    return _outLen;
}

int SendChunk(Connection con, Uint32 bytes, int flush) {
    if (bytes > _outLen)
        bytes = _outLen;

    if (con && SendRaw(con, _outgoing, bytes) == -1)
        return -1;
    
    if (flush) {
        memmove(_outgoing, _outgoing + bytes, _outLen - bytes);
        
        _outLen -= bytes;
        if (_outLen == 0) {
            free(_outgoing);
            _outgoing = NULL;
        }
    }
    
    return 0;
}

int ReceiveChunk(Connection con, Uint32 bytes) {
    if (!_incoming) {
        _inAlloc = 4;
        _inLen = 0;
        _inFront = 0;
        _incoming = (Uint8 *)malloc(_inAlloc);
    }
    
    memmove(_incoming, _incoming + _inFront, _inLen);
    _inFront = 0;
    
    while (_inLen + bytes > _inAlloc) {
        _inAlloc *= 2;
        _incoming = (Uint8 *)realloc(_incoming, _inAlloc);
    }
    
    if (ReceiveRaw(con, _incoming + _inLen, bytes) == -1)
        return -1;

    fflush(stdout);
    _inLen += bytes;
    
    return 0;
}

}

