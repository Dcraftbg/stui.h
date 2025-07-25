#define STUI_IMPLEMENTATION
#include "../stui.h"

int main(void) {
    size_t w, h;
    stui_term_get_size(&w, &h);
    stui_setsize(w, h);
    stui_clear();

    for(size_t y = 0; y < h; ++y) {
        for(size_t x = 0; x < w; ++x) {
            uint8_t r = ((float)x / (float)w) * 255.0;
            uint8_t g = (1.0 - ((float)y / (float)h)) * 255.0;
            stui_putchar_color(x, y, ' ', 0, STUI_RGB((((uint32_t)r) << 16) | (((uint32_t)g) << 8) | 0));
        }
    }
    stui_refresh();
    getchar();
}
