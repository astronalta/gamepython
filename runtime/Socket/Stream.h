/*
 * Copyright (c) 2010 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef JOGO_SOCKET_STREAM_H
#define JOGO_SOCKET_STREAM_H

#include "Primitives.h"
#include "Io/Stream.h"
#include "Socket/Socket.h"
#include "Coroutine.h"

typedef struct Socket_Stream* Socket_Stream;
struct Socket_Stream {
    VoidPtr _vtable;
    U64 _refcount;
    Io_Stream stream;
    struct Socket_Addr addr;
    struct Socket_Addr peer;
};

Socket_Stream Socket_Stream__init();
void Socket_Stream_close(Socket_Stream self);
void Socket_Stream_peer__s(Socket_Stream self, Socket_Addr addr);
void Socket_Stream_connect(Socket_Stream self);

#endif
