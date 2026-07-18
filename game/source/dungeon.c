// dungeon.c — dungeon map data, procedural generation, and rendering.
// Does NOT call libxenon directly — only goes through render_draw_tile(),
// per the platform-abstraction rule in NOTES.md.
//
// Generation algorithm (random room placement + L-shaped corridors) and
// overlap/connection logic validated standalone in dungen_test/ before
// being ported here — see NOTES.md.

#include "dungeon.h"
#include "render.h"
#include <stdlib.h>

#define MAX_ROOMS       5
#define ROOM_MIN_SIZE   10
#define ROOM_MAX_SIZE   16
#define ROOM_PADDING    1   // min gap enforced between rooms when checking overlap
#define CORRIDOR_WIDTH  2   // tiles wide; carved from the connecting line
                            // downward/rightward, not centered — accepted
                            // simplification for now

typedef struct {
    int x, y, w, h;
} room_t;

static tile_type_t map[DUNGEON_HEIGHT][DUNGEON_WIDTH];
static room_t rooms[MAX_ROOMS];
static int room_count;

static void map_fill_walls(void) {
    for (int y = 0; y < DUNGEON_HEIGHT; y++) {
        for (int x = 0; x < DUNGEON_WIDTH; x++) {
            map[y][x] = TILE_WALL;
        }
    }
}

static int room_overlaps(room_t *a, room_t *b) {
    // Padding expands 'a' by ROOM_PADDING on each side before checking —
    // keeps a visible wall gap between adjacent rooms rather than rooms
    // touching edge-to-edge.
    return !(a->x + a->w + ROOM_PADDING <= b->x ||
             b->x + b->w + ROOM_PADDING <= a->x ||
             a->y + a->h + ROOM_PADDING <= b->y ||
             b->y + b->h + ROOM_PADDING <= a->y);
}

static void carve_room(room_t *r) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            map[y][x] = TILE_FLOOR;
        }
    }
}

static void room_center(room_t *r, int *cx, int *cy) {
    *cx = r->x + r->w / 2;
    *cy = r->y + r->h / 2;
}

static void carve_h_corridor(int x1, int x2, int y) {
    int lo = x1 < x2 ? x1 : x2;
    int hi = x1 < x2 ? x2 : x1;
    for (int x = lo; x <= hi; x++) {
        for (int wy = 0; wy < CORRIDOR_WIDTH; wy++) {
            int yy = y + wy;
            if (yy >= 0 && yy < DUNGEON_HEIGHT) map[yy][x] = TILE_FLOOR;
        }
    }
}

static void carve_v_corridor(int y1, int y2, int x) {
    int lo = y1 < y2 ? y1 : y2;
    int hi = y1 < y2 ? y2 : y1;
    for (int y = lo; y <= hi; y++) {
        for (int wx = 0; wx < CORRIDOR_WIDTH; wx++) {
            int xx = x + wx;
            if (xx >= 0 && xx < DUNGEON_WIDTH) map[y][xx] = TILE_FLOOR;
        }
    }
}

static void connect_rooms(room_t *a, room_t *b) {
    int ax, ay, bx, by;
    room_center(a, &ax, &ay);
    room_center(b, &bx, &by);

    // Randomize corridor bend order for visual variety, rather than
    // always going horizontal-then-vertical.
    if (rand() % 2) {
        carve_h_corridor(ax, bx, ay);
        carve_v_corridor(ay, by, bx);
    } else {
        carve_v_corridor(ay, by, ax);
        carve_h_corridor(ax, bx, by);
    }
}

void dungeon_init(unsigned int seed) {
    srand(seed);
    map_fill_walls();
    room_count = 0;

    int attempts = 0;
    int max_attempts = MAX_ROOMS * 20; // avoid infinite loop if placement gets unlucky

    while (room_count < MAX_ROOMS && attempts < max_attempts) {
        attempts++;

        int w = ROOM_MIN_SIZE + (rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1));
        int h = ROOM_MIN_SIZE + (rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE + 1));
        int x = 1 + (rand() % (DUNGEON_WIDTH - w - 2));
        int y = 1 + (rand() % (DUNGEON_HEIGHT - h - 2));

        room_t candidate = { x, y, w, h };

        int overlap = 0;
        for (int i = 0; i < room_count; i++) {
            if (room_overlaps(&candidate, &rooms[i])) { overlap = 1; break; }
        }
        if (overlap) continue;

        carve_room(&candidate);
        rooms[room_count] = candidate;

        if (room_count > 0) {
            connect_rooms(&rooms[room_count - 1], &rooms[room_count]);
        }

        room_count++;
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

void dungeon_get_start(dungeon_point_t *out) {
    // Falls back to map center if generation somehow placed zero rooms
    // (shouldn't happen given max_attempts headroom, but avoids handing
    // back garbage/uninitialized coordinates).
    if (room_count == 0) {
        out->x = DUNGEON_WIDTH / 2;
        out->y = DUNGEON_HEIGHT / 2;
        return;
    }
    room_center(&rooms[0], &out->x, &out->y);
}

void dungeon_get_end(dungeon_point_t *out) {
    if (room_count == 0) {
        out->x = DUNGEON_WIDTH / 2;
        out->y = DUNGEON_HEIGHT / 2;
        return;
    }
    room_center(&rooms[room_count - 1], &out->x, &out->y);
}

int dungeon_get_points_of_interest(dungeon_point_t *out, int max_out) {
    int count = 0;
    // Rooms 1 .. room_count-2 (excludes room 0 = start, room_count-1 = end).
    for (int i = 1; i < room_count - 1 && count < max_out; i++) {
        room_center(&rooms[i], &out[count].x, &out[count].y);
        count++;
    }
    return count;
}
