// render.c — screen rendering layer. Only file (besides input.c) allowed
// to call libxenon directly, per NOTES.md's platform-abstraction rule.

#include "render.h"
#include <console/console.h>
#include <xenos/xenos.h>

static unsigned int pack_bgra(unsigned char r, unsigned char g, unsigned char b) {
    // libxenon's console color format is BGRA, alpha byte unused (0).
    return ((unsigned int)b << 24) | ((unsigned int)g << 16) | ((unsigned int)r << 8);
}

void render_init(void) {
    xenos_init(VIDEO_MODE_AUTO);
    console_init();
}

void render_clear(void) {
    console_clrscr();
}

void render_draw_tile(int x, int y, char glyph,
                       unsigned char fg_r, unsigned char fg_g, unsigned char fg_b,
                       unsigned char bg_r, unsigned char bg_g, unsigned char bg_b) {
    console_set_colors(pack_bgra(bg_r, bg_g, bg_b), pack_bgra(fg_r, fg_g, fg_b));
    console_set_cursor(x, y);
    console_putch(glyph);
}

void render_get_grid_size(unsigned int *cols, unsigned int *rows) {
    console_get_dimensions(cols, rows);
}
