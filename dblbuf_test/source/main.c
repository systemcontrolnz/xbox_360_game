// dblbuf_test/source/main.c — Step 2: flip to a solid-color backbuffer.
// Fills the ENTIRE backbuffer with one uniform color before flipping, which
// sidesteps needing to know Xenos's tiled/swizzled pixel layout (a uniform
// fill looks correct regardless of tiling -- that's Step 3's problem).
// Confirms the flip produces a clean, unambiguous, full-screen visual change.

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

int main(void) {
    xenos_init(VIDEO_MODE_AUTO);
    console_init();

    printf("\n--- Double buffer flip test (Step 2: solid fill) ---\n");

    struct ati_info *ai = (struct ati_info*)0xec806100ULL;
    unsigned int fb_phys  = ai->base;
    unsigned char *fb_cpu = (unsigned char*)(unsigned long)(fb_phys | 0x80000000);

    unsigned int fb_w = ((ai->width + 31) >> 5) << 5;
    unsigned int fb_h = ((ai->height + 31) >> 5) << 5;
    unsigned int fb_bytes = fb_w * fb_h * 4;

    printf("Primary FB: phys=0x%08x cpu=%p size=%u bytes\n", fb_phys, fb_cpu, fb_bytes);

    unsigned char *backbuf = memalign(4096, fb_bytes);
    if (!backbuf) {
        printf("backbuf alloc failed!\n");
        while (1) {}
    }
    unsigned int backbuf_phys = ((unsigned long)backbuf) & ~0x80000000UL;
    printf("Backbuffer: phys=0x%08x cpu=%p\n", backbuf_phys, backbuf);

    // Fill the WHOLE backbuffer with one solid color (BGRA-packed bright
    // green, matching render.c's pack_bgra format: b<<24 | g<<16 | r<<8).
    // A uniform fill is tiling-agnostic -- every byte is identical either way.
    unsigned int green = (0u << 24) | (255u << 16) | (0u << 8);
    unsigned int *fill_ptr = (unsigned int*)backbuf;
    unsigned int fill_count = fb_bytes / 4;
    for (unsigned int i = 0; i < fill_count; i++) {
        fill_ptr[i] = green;
    }
    memdcbst(backbuf, fb_bytes);

    printf("Flipping to solid-green backbuffer in ~3 seconds...\n");
    for (volatile int i = 0; i < 300000000; i++) {}

    // --- Flip to backbuffer ---
    xenos_write32(D1GRPH_UPDATE, 1);
    xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, backbuf_phys);
    xenos_write32(D1GRPH_UPDATE, 0);

    for (volatile int i = 0; i < 900000000; i++) {}

    // --- Flip back to primary (still has our original printf'd text) ---
    xenos_write32(D1GRPH_UPDATE, 1);
    xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, fb_phys);
    xenos_write32(D1GRPH_UPDATE, 0);

    printf("Flipped back. If you can read this, the flip worked!\n");

    while (1) {}
    return 0;
}
