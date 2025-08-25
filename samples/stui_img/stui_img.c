#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STUI_IMPLEMENTATION
#include "../../stui.h"

int main(void) {
    size_t w, h;
    stui_term_get_size(&w, &h);
    stui_setsize(w, h);
    stui_clear();

    int img_w, img_h, img_comp;
    uint32_t* pixels = (uint32_t*)stbi_load("./Jimbo.jpg", &img_w, &img_h, &img_comp, 4);
    if(!pixels) return 1;

    for(size_t y = 0; y < h; ++y) {
        for(size_t x = 0; x < w; ++x) {
            float u = ((float)x / (float)w);
            float v = (((float)y / (float)h));

            int img_x = (u * img_w);
            int img_y = (v * img_h);

            // ABGR
            uint32_t pixel = pixels[img_y * img_w + img_x];
            uint32_t color = ((pixel & 0xFF) << 16) | 
                             (((pixel >> 8) & 0xFF) << 8) |
                             (((pixel >> 16) & 0xFF) << 0);
            stui_putchar_color(x, y, ' ', 0, STUI_RGB(color));
        }
    }
    stui_refresh();
}
