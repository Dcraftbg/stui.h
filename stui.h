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

// CORE:
void stui_setsize(size_t x, size_t y);
enum {
    STUI_COLOR_KIND_RST,
    STUI_COLOR_KIND_RGB,
    STUI_COLOR_KIND_COUNT,
};
#define STUI_COLOR_KIND_SHIFT 24
#define STUI_RGB(clr) ((STUI_COLOR_KIND_RGB << STUI_COLOR_KIND_SHIFT) | ((clr) & 0xFFFFFF))
#define STUI_GET_COLOR_KIND(color) ((color) >> STUI_COLOR_KIND_SHIFT)

void stui_putchar_color(size_t x, size_t, int c, uint32_t fg, uint32_t bg);
static void stui_putchar(size_t x, size_t y, int c) {
    stui_putchar_color(x, y, c, 0, 0);
}
void stui_refresh(void);
// Raw API
void stui_goto(size_t x, size_t y);
void stui_clear(void);
// Terminal API
// stui Terminal interface
// Character echo
#define STUI_TERM_FLAG_ECHO    (1 << 0)
// Instant or non-canonical mode
#define STUI_TERM_FLAG_INSTANT (1 << 1)
typedef uint8_t stui_term_flag_t;

void stui_term_get_size(size_t *w, size_t *h);
stui_term_flag_t stui_term_get_flags(void);
void stui_term_set_flags(stui_term_flag_t flags);
// Helper functions
static void stui_term_enable_echo(void) {
    stui_term_set_flags(stui_term_get_flags() | STUI_TERM_FLAG_ECHO);
}
static void stui_term_disable_echo(void) {
    stui_term_set_flags(stui_term_get_flags() & ~STUI_TERM_FLAG_ECHO);
}
static void stui_term_enable_instant(void) {
    stui_term_set_flags(stui_term_get_flags() | STUI_TERM_FLAG_INSTANT);
}
static void stui_term_disable_instant(void) {
    stui_term_set_flags(stui_term_get_flags() & ~STUI_TERM_FLAG_INSTANT);
}

// UI thingies
void stui_window_border(size_t x, size_t y, size_t w, size_t h, int tb, int lr, int corner);


#ifdef STUI_IMPLEMENTATION
#define STUI_BUFFER_COUNT 2
static uint8_t _stui_back_buffer = 0; 
typedef struct {
    uint8_t code;
#ifndef STUI_NO_COLORS
    uint32_t fg;
    uint32_t bg;
#endif
} _StuiCodepoint;
static _StuiCodepoint* _stui_buffers[STUI_BUFFER_COUNT] = { 0 };
static size_t _stui_width = 0, _stui_height = 0;
void stui_setsize(size_t x, size_t y) {
    _stui_width = x;
    _stui_height = y;
    for(size_t i = 0; i < STUI_BUFFER_COUNT; ++i) {
        _stui_buffers[i] = STUI_REALLOC(_stui_buffers[i], x*y*sizeof(*_stui_buffers[0]));
        assert(_stui_buffers[i]);
        for(size_t j = 0; j < x*y; ++j) {
            _stui_buffers[i][j].code = ' ';
#ifndef STUI_NO_COLORS
            _stui_buffers[i][j].fg = _stui_buffers[i][j].bg = 0;
#endif
        }
    }
}
void stui_putchar_color(size_t x, size_t y, int c, uint32_t fg, uint32_t bg) {
    _StuiCodepoint* buffer = _stui_buffers[_stui_back_buffer];
    assert(x < _stui_width);
    assert(y < _stui_height);
    buffer[y * _stui_width + x].code = c;
#ifdef STUI_NO_COLORS
    (void)fg;
    (void)bg;
#else
    buffer[y * _stui_width + x].fg = fg;
    buffer[y * _stui_width + x].bg = bg;
#endif
}

static void _stui_set_color(uint32_t color, uint8_t off) {
    uint8_t kind = STUI_GET_COLOR_KIND(color);
    assert(kind < STUI_COLOR_KIND_COUNT); 
    static_assert(STUI_COLOR_KIND_COUNT == 2, "Update color kinds");
    switch(kind) {
    case STUI_COLOR_KIND_RST:
        printf("\033[%dm", 39 + off);
        break;
    case STUI_COLOR_KIND_RGB:
        printf("\033[%d;2;%d;%d;%dm", 38 + off, (color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF);
        break;
    }
}
void stui_refresh(void) {
    _StuiCodepoint* back  = _stui_buffers[_stui_back_buffer];
    _StuiCodepoint* front = _stui_buffers[(_stui_back_buffer + 1) % STUI_BUFFER_COUNT];
    uint32_t fg = 0, bg = 0;
    printf("\033[0m");
    for(size_t i = 0; i < _stui_width*_stui_height; ++i) {
        if(back[i].code != front[i].code 
        #ifndef STUI_NO_COLORS
            || back[i].fg != front[i].fg 
            || back[i].bg != front[i].bg
        #endif
        ) {
#ifndef STUI_NO_COLORS
            if(back[i].fg != fg) {
                _stui_set_color(back[i].fg, 0);
            }
            if(back[i].bg != bg) {
                _stui_set_color(back[i].bg, 10);
            }
            fg = back[i].fg;
            bg = back[i].bg;
#endif
            size_t x = i % _stui_width, y = i / _stui_width;
            stui_goto(x, y);
            printf("%c", back[i].code);
            front[i] = back[i];
        }
    }
    printf("\033[0m");
    fflush(stdout);
    _stui_back_buffer = (_stui_back_buffer + 1) % STUI_BUFFER_COUNT;
}

// Raw API
void stui_goto(size_t x, size_t y) {
    printf("\033[%zu;%zuH", y+1, x+1);
    fflush(stdout);
}
void stui_clear(void) {
    printf("\033[2J");
    printf("\033[H");
    fflush(stdout);
}

// Utilities
void stui_window_border(size_t x, size_t y, size_t w, size_t h, int tb, int lr, int corner) {
    for(size_t dx = x + 1; dx < (x + w); dx++) {
        stui_putchar(dx, y  , tb);
        stui_putchar(dx, y+h, tb);
    }
    for(size_t dy = y + 1; dy < (y + h); dy++) {
        stui_putchar(x    , dy, lr);
        stui_putchar(x + w, dy, lr);
    }
    stui_putchar(x    , y    , corner);
    stui_putchar(x + w, y    , corner);
    stui_putchar(x    , y + h, corner);
    stui_putchar(x + w, y + h, corner);
}

// Terminal API
#ifdef _MINOS
# include <minos/tty/tty.h>
#elif _WIN32
# include <Windows.h>
#else
# include <termios.h>
# include <unistd.h>
# include <signal.h>
# include <sys/ioctl.h>
#endif

void stui_term_get_size(size_t *w, size_t *h) {
#ifdef _MINOS
    *w = 80;
    *h = 24;
#elif _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    *w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize winsz;
    ioctl(0, TIOCGWINSZ, &winsz);
    *w = winsz.ws_col;
    *h = winsz.ws_row;
#endif
}
stui_term_flag_t stui_term_get_flags(void) {
    stui_term_flag_t flags = 0;
#ifdef _MINOS
    ttyflags_t tty_flags;
    tty_get_flags(fileno(stdin), &tty_flags);
    if(tty_flags & TTY_ECHO)    flags |= STUI_TERM_FLAG_ECHO;
    if(tty_flags & TTY_INSTANT) flags |= STUI_TERM_FLAG_INSTANT;
#elif _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    if (mode & ENABLE_ECHO_INPUT) flags |= STUI_TERM_FLAG_ECHO;
    if (!(mode & ENABLE_LINE_INPUT)) flags |= STUI_TERM_FLAG_INSTANT;
#else
    // Assume Unix platform
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    if(term.c_lflag & ECHO)      flags |= STUI_TERM_FLAG_ECHO;
    if(!(term.c_lflag & ICANON)) flags |= STUI_TERM_FLAG_INSTANT;
#endif
    return flags;
}
void stui_term_set_flags(stui_term_flag_t flags) {
#ifdef _MINOS
    ttyflags_t tty_flags;
    tty_get_flags(fileno(stdin), &tty_flags);
    tty_flags &= ~(TTY_ECHO | TTY_INSTANT);
    if(flags & STUI_TERM_FLAG_ECHO) tty_flags |= TTY_ECHO;
    if(flags & STUI_TERM_FLAG_INSTANT) tty_flags |= TTY_INSTANT;
    tty_set_flags(fileno(stdin), tty_flags);
#elif _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    if (flags & STUI_TERM_FLAG_ECHO) 
        mode |= ENABLE_ECHO_INPUT;
    if (!(flags & STUI_TERM_FLAG_INSTANT)) 
        mode |= ENABLE_LINE_INPUT;
    SetConsoleMode(hStdin, mode);
#else
    // Assume Unix platform
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ECHO | ICANON);
    if(flags & STUI_TERM_FLAG_ECHO) term.c_lflag |= ECHO;
    if(!(flags & STUI_TERM_FLAG_INSTANT)) term.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}
#endif
