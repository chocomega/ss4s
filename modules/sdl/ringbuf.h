#pragma once

#include <stddef.h>

typedef struct sdlaud_ringbuf_t sdlaud_ringbuf;

sdlaud_ringbuf *sdlaud_ringbuf_new(size_t capacity);

size_t sdlaud_ringbuf_write(sdlaud_ringbuf *buf, const unsigned char *src, size_t size);

size_t sdlaud_ringbuf_read(sdlaud_ringbuf *buf, unsigned char *dst, size_t size);

size_t sdlaud_ringbuf_clear(sdlaud_ringbuf *buf);

size_t sdlaud_ringbuf_size(const sdlaud_ringbuf *buf);

void sdlaud_ringbuf_delete(sdlaud_ringbuf *buf);