// dblbuf_test/source/main.c — Step 3: draw shaped content using the Xenos
// tiled-framebuffer addressing formula (reverse-derived from console.c's
// console_pset32), generalized to work on an arbitrary buffer + width
// rather than the hardcoded static console_fb. Confirms the formula
// generalizes correctly by drawing something with a KNOWN position, not
// just a uniform fill (which would look correct even if tiling were wrong).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xenos/xenos.h>
#include <console/console.h>
#include <ppc/cache.h>

extern void xenos_write32(int reg, unsigned int val);
extern void *memalign(unsigned int alignment, unsigned int size);

struct ati_info {
    unsigned int unknown1[4];
    unsigned int base;
    unsigned int unknown2[8];
    unsigned int width;
    unsigned int height;
} __attribute__((packed));

// Same tiled-addressing formula as console.c's console_pset32, generalized
// to take an explicit buffer pointer and width instead of the hardcoded
// static console_fb.
static inline void buf_pset32(unsigned int *buf, unsigned int width, int x, int y, unsigned int color) {
    unsigned int base = (((y >> 5) * 32 * width + ((x >> 5) << 10)
        + (x & 3) + ((y & 1) << 2) + (((x & 31) >> 2) << 3) + (((y & 31) >> 1) << 6))
        ^ ((y & 8) << 2));
    buf[base] = color;
}

int main(void) {
    xenos_init(VIDEO_MODE_AUTO);
    console_init();

    printf("\n--- Double buffer flip test (Step 3: shaped draw) ---\n");

    struct ati_info *ai = (struct ati_info*)0xec806100ULL;
    unsigned int fb_phys  = ai->base;
    unsigned char *fb_cpu = (unsigned char*)(unsigned long)(fb_phys | 0x80000000);

    unsigned int fb_w = ((ai->width + 31) >> 5) << 5;
    unsigned int fb_h = ((ai->height + 31) >> 5) << 5;
    unsigned int fb_bytes = fb_w * fb_h * 4;

    printf("Primary FB: phys=0x%08x cpu=%p w=%u h=%u\n", fb_phys, fb_cpu, fb_w, fb_h);

    unsigned char *backbuf = memalign(4096, fb_bytes);
    if (!backbuf) {
        printf("backbuf alloc failed!\n");
        while (1) {}
    }
    unsigned int backbuf_phys = ((unsigned long)backbuf) & ~0x80000000UL;
    printf("Backbuffer: phys=0x%08x cpu=%p\n", backbuf_phys, backbuf);

    unsigned int *pixels = (unsigned int*)backbuf;

    // Dark blue background, BGRA-packed: b=80, g=0, r=0
    unsigned int bg_color = (80u << 24) | (0u << 16) | (0u << 8);
    for (unsigned int y = 0; y < fb_h; y++) {
        for (unsigned int x = 0; x < fb_w; x++) {
            buf_pset32(pixels, fb_w, x, y, bg_color);
        }
    }

    // Bright red rectangle at a KNOWN position: x=100..300, y=100..200.
    // Should appear well inside the top-left area, clearly offset from
    // both edges -- easy to eyeball whether it lands in the right spot.
    unsigned int red_color = (0u << 24) | (0u << 16) | (255u << 8);
    for (unsigned int y = 100; y < 200; y++) {
        for (unsigned int x = 100; x < 300; x++) {
            buf_pset32(pixels, fb_w, x, y, red_color);
        }
    }

    memdcbst(backbuf, fb_bytes);

    printf("Backbuffer drawn: dark blue bg + red rect at x=100-300,y=100-200\n");
    printf("Flipping in ~3 (uncalibrated) seconds...\n");
    for (volatile int i = 0; i < 300000000; i++) {}

    xenos_write32(D1GRPH_UPDATE, 1);
    xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, backbuf_phys);
    xenos_write32(D1GRPH_UPDATE, 0);

    for (volatile int i = 0; i < 900000000; i++) {}

    xenos_write32(D1GRPH_UPDATE, 1);
    xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, fb_phys);
    xenos_write32(D1GRPH_UPDATE, 0);

    printf("Flipped back. If the rectangle was in the right place, tiling works!\n");

    while (1) {}
    return 0;
}
