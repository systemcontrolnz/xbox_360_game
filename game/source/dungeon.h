// dungeon.h — dungeon map data, procedural generation, and rendering.
// Does NOT call libxenon directly — only goes through render.h, per the
// platform-abstraction rule in NOTES.md.

#ifndef DUNGEON_H
#define DUNGEON_H

typedef enum {
    TILE_FLOOR,
    TILE_WALL
} tile_type_t;

#define DUNGEON_WIDTH   78
#define DUNGEON_HEIGHT  44

// A generic anchor point in the dungeon — currently just a room center.
// Used as the basis for the start point, end point, and points of
// interest below. Deliberately generic/untyped for now: what a POI
// *represents* (collectible, boss, optional objective, etc.) is a
// Milestone 3 step 5 concern, once monsters/objectives exist to assign
// to these points. This step only exposes *where* points exist.
typedef struct {
    int x, y;
} dungeon_point_t;

// Generates a new procedural dungeon (random room placement + L-shaped
// corridors) using the given seed. Same seed always produces the same
// layout. Seed is currently passed in directly by the caller (main.c
// uses a fixed placeholder) — a future main menu will let the player
// enter a seed manually or randomize one before calling this, per
// Design point 6 in NOTES.md; dungeon.c itself has no menu/UI concerns.
void dungeon_init(unsigned int seed);

// Returns the tile type at (x, y). Out-of-bounds coordinates return
// TILE_WALL, so callers (e.g. collision code) can query freely without
// bounds-checking first.
tile_type_t dungeon_get_tile(int x, int y);

// Draws the full map via render_draw_tile(). Call once per frame, after
// render_clear() and before drawing entities on top.
void dungeon_render(void);

// Player's starting position for this dungeon (center of the first room
// placed). Always lands on floor.
void dungeon_get_start(dungeon_point_t *out);

// Level exit position for this dungeon (center of the last room placed).
// Always lands on floor. Distinct from start as long as more than one
// room was generated.
void dungeon_get_end(dungeon_point_t *out);

// Fills 'out' (capacity 'max_out') with points of interest — the
// centers of all rooms EXCLUDING the start and end rooms. Intended as
// candidate anchor points for future objective placement (collectibles,
// bosses, optional objectives — Milestone 3 step 5 / Design point 6).
// Returns the actual count written (may be less than max_out, and may
// be 0 if fewer than 3 rooms were generated).
int dungeon_get_points_of_interest(dungeon_point_t *out, int max_out);

#endif // DUNGEON_H
