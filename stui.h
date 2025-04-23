#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifndef STUI_REALLOC
# include <stdlib.h>
# define STUI_REALLOC realloc
#endif

void stui_setsize(size_t x, size_t y);
void stui_putchar(size_t x, size_t y, int c);
void stui_refresh(void);

#ifdef STUI_IMPLEMENTATION
#define STUI_BUFFER_COUNT 2
static uint8_t _stui_back_buffer = 0; 
static char* _stui_buffers[STUI_BUFFER_COUNT] = { 0 };
static size_t _stui_width = 0, _stui_height = 0;
void stui_setsize(size_t x, size_t y) {
    _stui_width = x;
    _stui_height = y;
    size_t n = x*y*sizeof(_stui_buffers[0]);
    for(size_t i = 0; i < STUI_BUFFER_COUNT; ++i) {
        _stui_buffers[i] = STUI_REALLOC(_stui_buffers[i], n);
        assert(_stui_buffers[i]);
        memset(_stui_buffers[i], ' ', n);
    }
}
void stui_putchar(size_t x, size_t y, int c) {
    char* buffer = _stui_buffers[_stui_back_buffer];
    assert(x < _stui_width);
    assert(y < _stui_height);
    buffer[y * _stui_width + x] = c;
}
void stui_refresh(void) {
    char* back  = _stui_buffers[_stui_back_buffer];
    char* front = _stui_buffers[(_stui_back_buffer + 1) % STUI_BUFFER_COUNT];
    for(size_t i = 0; i < _stui_width*_stui_height; ++i) {
        if(back[i] != front[i]) {
            size_t x = i % _stui_width, y = i / _stui_width;
            printf("\033[%zu;%zuH", y+1, x+1);
            printf("%c", back[i]);
            fflush(stdout);
            front[i] = back[i];
        }
    }
    _stui_back_buffer = (_stui_back_buffer + 1) % STUI_BUFFER_COUNT;
}
#endif
