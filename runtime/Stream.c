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

#include "Stream.h"
#include "String.h"
#ifndef WINDOWS
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>

void Stream_read(Stream self, Buffer buffer) {
#ifndef WINDOWS
    Byte* buf = buffer->data + buffer->begin;
    Int len = buffer->capacity - buffer->end;
    Int ret = read(self->handle, buf, len);
    if (ret == 0) {
        // EOF
        return;
    } else if (ret == -1) {
        // ERROR
        return;
    } else {
        buffer->end += ret;
    }
#endif
}

void Stream_write(Stream self, Buffer buffer) {
#ifndef WINDOWS
    Byte* buf = buffer->data + buffer->begin;
    Int len = buffer->end - buffer->begin;
    Int ret = write(self->handle, buf, len);
    if (ret == 0) {
        return;
    } else if (ret == -1) {
        // ERROR
        return;
    } else {
        buffer->begin += ret;
    }
#endif
}

Int Stream_get(Stream self) {
    Buffer buf = self->read_buf;
    if (buf->begin == buf->end) {
        buf->begin = 0;
        buf->end = 0;
        Stream_read(self, buf);
    }
    if (buf->begin >= buf->end) {
   printf("end");
        return -1;
    } else {
        return buf->data[buf->begin++];
    }
}

Int Stream_peek(Stream self) {
    Buffer buf = self->read_buf;
    if (buf->begin == buf->end) {
        buf->begin = 0;
        buf->end = 0;
        Stream_read(self, buf);
    }
    if (buf->begin >= buf->end) {
        return -1;
    } else {
        return buf->data[buf->begin];
    }
}

void Stream_put(Stream self, Char ch) {
    Buffer buf = self->write_buf;
    if (buf->end == buf->capacity) {
        Stream_write(self, buf);
    }
    if (buf->end >= buf->capacity) {
        return;
    } else {
        buf->data[buf->end++] = ch;
    }
}

String Stream_scan(Stream self, String delim) {
    // Read in characters until one of the characters in 'delim' is found,
    // then return all the characters read so far.
    Int length = 16;
    String ret = malloc(sizeof(struct String) + length + 1); 
    if (!ret) {
        fprintf(stderr, "Out of memory");
        fflush(stderr);
        abort();
    }
    ret->_vtable = String__vtable;
    ret->_refcount = 1;
    ret->length = 0;

    while (1) {
        // Loop until we find a delimiter somewhere in the input stream
        Char next = Stream_get(self);
        for (Char* c = delim->data; *c; ++c) {
            if (*c == next) {
                ret->data[ret->length] = '\0';
                return ret;
            }
        }
        // Resize the string if necessary
        if (ret->length >= length) {
            length *= 2;
            String exp = malloc(sizeof(struct String) + length + 1);
            if (!ret) {
                fprintf(stderr, "Out of memory");
                fflush(stderr);
                abort();
            }
            exp->_vtable = String__vtable;
            exp->_refcount = 1;
            exp->length = ret->length;
            Char* c = exp->data;
            for (Int i = 0; i < ret->length; i++) {
                *c++ = ret->data[i];
            } 
        }
        // Append a new character
        ret->data[ret->length++] = next;
    } 

    return 0;
}

void Stream_print(Stream self, String str) {
    for (Int i = 0; i < str->length; i++) {
        Stream_put(self, str->data[i]);
    }
}

void Stream_flush(Stream self) {
    Stream_write(self, self->write_buf); 
}

void Stream_close(Stream self) {
    Stream_flush(self);
    self->write_buf->begin = self->write_buf->end = 0;
    self->read_buf->begin = self->read_buf->end = 0;
#ifndef WINDOWS
    Int ret = close(self->handle);
    if (ret == -1) {
        // Error
    }
#endif 
}

void println(String str) {
    // FIXME: Use Apollo streams
    if (str) {
        fwrite(str->data, str->length, 1, stdout);
    } else {
        fwrite("nil", 3, 1, stdout);
    }
    fwrite("\n", 1, 1, stdout);
    fflush(stdout);
}

void print(String str) {
    // FIXME: Use Apollo streams
    if (str) {
        fwrite(str->data, str->length, 1, stdout);
    } else {
        fwrite("nil", 3, 1, stdout);
    }
    fflush(stdout);
}
