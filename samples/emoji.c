#define STUI_IMPLEMENTATION
#include "../stui.h"

void character(size_t y, const char* name, int code) {
    size_t x = 0;
    while(*name) stui_putchar(x++, y, *name++);
    stui_putchar(x++, y, ' ');
    stui_putchar(x++, y, '-');
    stui_putchar(x++, y, '>');
    stui_putchar(x++, y, ' ');
    stui_putchar(x++, y, code);
}
int main(void) {
    size_t w, h;
    stui_term_get_size(&w, &h);
    stui_setsize(w, h);
    stui_clear();

    if(w < 16 || h < 3) {
        printf("Too small %zux%zu!\n", w, h);
        return 0;
    }
    character(0, "Party", 0x1F973);
    character(1, "Sunglasses", 0x1F60E);
    character(2, "Melting", 0x1FAE0);
    stui_refresh();
}
