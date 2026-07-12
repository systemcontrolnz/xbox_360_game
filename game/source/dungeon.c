// dungeon.c — dungeon map data and rendering. Does NOT call libxenon
// directly — only goes through render_draw_tile(), per the platform-
// abstraction rule in NOTES.md.

#include "dungeon.h"
#include "render.h"

static tile_type_t map[DUNGEON_HEIGHT][DUNGEON_WIDTH];

// Fixed hand-authored test layout: a single rectangular room, walls
// around the border. Milestone 3 step 1 — static map only, no collision
// or procedural generation yet (those are steps 2 and 4).
void dungeon_init(void) {
    for (int y = 0; y < DUNGEON_HEIGHT; y++) {
        for (int x = 0; x < DUNGEON_WIDTH; x++) {
            int border = (x == 0 || y == 0 ||
                          x == DUNGEON_WIDTH - 1 || y == DUNGEON_HEIGHT - 1);
            map[y][x] = border ? TILE_WALL : TILE_FLOOR;
        }
    }
}

tile_type_t dungeon_get_tile(int x, int y) {
    if (x < 0 || y < 0 || x >= DUNGEON_WIDTH || y >= DUNGEON_HEIGHT) {
        return TILE_WALL;
    }
    return map[y][x];
}

void dungeon_render(void) {
    for (int y = 0; y < DUNGEON_HEIGHT; y++) {
        for (int x = 0; x < DUNGEON_WIDTH; x++) {
            if (map[y][x] == TILE_WALL) {
                render_draw_tile(x, y, '#', 150, 150, 150, 0, 0, 0);
            } else {
                render_draw_tile(x, y, '.', 90, 90, 90, 0, 0, 0);
            }
        }
    }
}
