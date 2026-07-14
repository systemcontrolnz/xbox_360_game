// render.c — screen rendering layer. Only file (besides input.c) allowed
// to call libxenon directly, per NOTES.md's platform-abstraction rule.
//
// Double-buffered as of this revision: render_draw_tile() writes glyphs
// into an off-screen backbuffer only. Nothing reaches the display until
// render_present() flips the GPU's primary surface register, per the
// technique validated in dblbuf_test/ (see NOTES.md).

#include "render.h"
#include <console/console.h>
#include <xenos/xenos.h>
#include <ppc/cache.h>
#include <stdlib.h>

extern void xenos_write32(int reg, unsigned int val);
extern void *memalign(unsigned int alignment, unsigned int size);

// Glyph bitmap data (8x16, one byte per row, MSB-first) is DEFINED in
// libxenon's console.c (via font_8x16.h, GPL-licensed, sourced from the
// Linux kernel). We deliberately do NOT #include that header here:
//   1. It would vendor GPL source into this repo's tree unnecessarily.
//   2. The array has external linkage and is already defined in
//      libxenon.a (via console.c) — including it again here would be a
//      duplicate-symbol link error.
// This extern just references the symbol libxenon.a already exports.
// (Future: swapping in our own font data is a drop-in replacement of
// this one declaration + the linked-in array.)
extern const unsigned char fontdata_8x16[];

struct ati_info {
    unsigned int unknown1[4];
    unsigned int base;
    unsigned int unknown2[8];
    unsigned int width;
    unsigned int height;
} __attribute__((packed));

static unsigned int pack_bgra(unsigned char r, unsigned char g, unsigned char b) {
    // BGRA, alpha unused — same convention as libxenon's console_color[].
    return ((unsigned int)b << 24) | ((unsigned int)g << 16) | ((unsigned int)r << 8);
}

// on_screen_phys: physical address currently being scanned out by the GPU.
// backbuf_phys / backbuf: physical + CPU pointer for the buffer we draw
// into. render_present() swaps these roles each frame.
static unsigned int on_screen_phys;
static unsigned int backbuf_phys;
static unsigned int *backbuf;
static unsigned int fb_w, fb_h;   // tile-aligned pixel dimensions
static unsigned int fb_bytes;
static unsigned int grid_cols, grid_rows;

// Tiled-addressing formula reverse-derived from console.c's
// console_pset32, validated on hardware in dblbuf_test Step 3 — see
// NOTES.md. Generalized to take an explicit buffer pointer + width.
static inline void buf_pset32(unsigned int *buf, unsigned int width, int x, int y, unsigned int color) {
    unsigned int base = (((y >> 5) * 32 * width + ((x >> 5) << 10)
        + (x & 3) + ((y & 1) << 2) + (((x & 31) >> 2) << 3) + (((y & 31) >> 1) << 6))
        ^ ((y & 8) << 2));
    buf[base] = color;
}

#define font_pixel(ch, x, y) ((fontdata_8x16[(unsigned char)(ch) * 16 + (y)] >> (7 - (x))) & 1)

// Blits one 8x16 glyph into 'buf' at pixel position (px, py), top-left
// origin. Matches console.c's console_draw_char() pixel-for-pixel.
static void buf_draw_glyph(unsigned int *buf, unsigned int width, int px, int py,
                            char c, unsigned int fg, unsigned int bg) {
    for (int ly = 0; ly < 16; ly++) {
        for (int lx = 0; lx < 8; lx++) {
            buf_pset32(buf, width, px + lx, py + ly, font_pixel(c, lx, ly) ? fg : bg);
        }
    }
}

void render_init(void) {
    xenos_init(VIDEO_MODE_AUTO);
    console_init(); // kept for printf/debug text; not used for game rendering

    struct ati_info *ai = (struct ati_info *)0xec806100ULL;
    on_screen_phys = ai->base;

    fb_w = ((ai->width + 31) >> 5) << 5;
    fb_h = ((ai->height + 31) >> 5) << 5;
    fb_bytes = fb_w * fb_h * 4;

    backbuf = (unsigned int *)memalign(4096, fb_bytes);
    backbuf_phys = ((unsigned long)backbuf) & ~0x80000000UL;

    grid_cols = fb_w / 8;
    grid_rows = fb_h / 16;
}

void render_clear(void) {
    unsigned int bg = pack_bgra(0, 0, 0);
    unsigned int count = fb_w * fb_h;
    unsigned int *p = backbuf;
    // A plain linear fill is equivalent to a per-pixel tiled fill for a
    // uniform color — same reasoning as dblbuf_test Step 2's solid fill.
    while (count--) *p++ = bg;
}

void render_draw_tile(int x, int y, char glyph,
                       unsigned char fg_r, unsigned char fg_g, unsigned char fg_b,
                       unsigned char bg_r, unsigned char bg_g, unsigned char bg_b) {
    unsigned int fg = pack_bgra(fg_r, fg_g, fg_b);
    unsigned int bg = pack_bgra(bg_r, bg_g, bg_b);
    buf_draw_glyph(backbuf, fb_w, x * 8, y * 16, glyph, fg, bg);
}

void render_present(void) {
    memdcbst(backbuf, fb_bytes);

    xenos_write32(D1GRPH_UPDATE, 1);
    xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, backbuf_phys);
    xenos_write32(D1GRPH_UPDATE, 0);

    // Swap roles for next frame: the buffer that was just displayed
    // becomes the new draw target.
    unsigned int shown = backbuf_phys;
    backbuf_phys = on_screen_phys;
    on_screen_phys = shown;
    backbuf = (unsigned int *)((unsigned long)backbuf_phys | 0x80000000UL);
}

void render_get_grid_size(unsigned int *cols, unsigned int *rows) {
    *cols = grid_cols;
    *rows = grid_rows;
}
