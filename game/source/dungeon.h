// dungeon.h — dungeon map data and rendering.
// Does NOT call libxenon directly — only goes through render.h, per the
// platform-abstraction rule in NOTES.md.

#ifndef DUNGEON_H
#define DUNGEON_H

typedef enum {
    TILE_FLOOR,
    TILE_WALL
} tile_type_t;

#define DUNGEON_WIDTH  40
#define DUNGEON_HEIGHT 20

// Must be called once before any other dungeon_* call. Builds the fixed
// hand-authored test layout (not procedural yet — see NOTES.md Milestone 3
// step 4 for when procedural generation replaces this).
void dungeon_init(void);

// Returns the tile type at (x, y). Out-of-bounds coordinates return
// TILE_WALL, so callers (e.g. future collision code) can query freely
// without bounds-checking first.
tile_type_t dungeon_get_tile(int x, int y);

// Draws the full map via render_draw_tile(). Call once per frame, after
// render_clear() and before drawing entities on top.
void dungeon_render(void);

#endif // DUNGEON_H
