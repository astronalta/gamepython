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

#ifndef APOLLO_STREAM_H
#define APOLLO_STREAM_H

#include "Primitives.h"
#include "Io/Buffer.h"

typedef struct Io_Stream* Io_Stream;
struct Io_Stream {
    Ptr _vtable;
    U64 _refcount;
    Int handle;
    Io_Buffer read_buf;
    Io_Buffer write_buf;
    Int flags;
};

Io_Stream Io_Stream__init(Int handle);
void Io_Stream_read(Io_Stream self, Io_Buffer buffer);
void Io_Stream_write(Io_Stream self, Io_Buffer buffer);
Int Io_Stream_get(Io_Stream self);
Int Io_Stream_peek(Io_Stream self);
void Io_Stream_put(Io_Stream self, Char ch);
String Io_Stream_scan(Io_Stream self, String delim);
void Io_Stream_print(Io_Stream self, String str);
void Io_Stream_flush(Io_Stream self);
void Io_Stream_close(Io_Stream self);

#endif