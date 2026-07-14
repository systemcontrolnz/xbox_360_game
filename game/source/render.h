// render.h — public interface for the screen rendering layer.
// All libxenon-specific graphics calls are confined to render.c.
// No other file in this project should call libxenon directly — see the
// platform-abstraction rule in NOTES.md (this is what makes a future
// Windows/SDL2 port a matter of swapping this file, not rewriting logic).

#ifndef RENDER_H
#define RENDER_H

// Must be called once at startup, before any other render_* call.
// Handles video + console init internally.
void render_init(void);

// Clears the off-screen backbuffer. Call once at the start of each frame,
// before drawing tiles for that frame. Does NOT affect what's currently
// on screen until render_present() is called.
void render_clear(void);

// Draws a single glyph at grid cell (x, y) into the off-screen backbuffer.
// (0,0) is top-left. Color channels are 0-255. Not visible on screen
// until render_present() is called.
void render_draw_tile(int x, int y, char glyph,
                       unsigned char fg_r, unsigned char fg_g, unsigned char fg_b,
                       unsigned char bg_r, unsigned char bg_g, unsigned char bg_b);

// Flips the completed backbuffer to the display (double buffering).
// Call once per frame, after all render_draw_tile() calls for that frame.
void render_present(void);

// Size of the character grid (columns, rows) — not pixels.
void render_get_grid_size(unsigned int *cols, unsigned int *rows);

#endif // RENDER_H
