// monsters.c — see monsters.h for design notes.

#include "monsters.h"
#include "dungeon.h"
#include "render.h"

#define MONSTER_HP 5   // placeholder — not tuned, see NOTES.md conventions
#define AGGRO_RANGE_PX  128   // placeholder, not tuned — 8 tiles at 16px pitch
#define MONSTER_MOVE_INTERVAL 4   // monsters only step on 1 of every N calls
                                   // to monsters_update() — quarter player
                                   // speed by throttling frequency, not by
                                   // fractional pixels (move_speed is int)

static monster_t monsters[MAX_MONSTERS];
static int monster_count;

void monsters_init(void) {
    dungeon_point_t points[MAX_MONSTERS];
    int n = dungeon_get_points_of_interest(points, MAX_MONSTERS);
    unsigned int tile_px = render_get_tile_pitch_px();

    monster_count = n;
    for (int i = 0; i < n; i++) {
        entity_init(&monsters[i].base,
                    (int)(points[i].x * tile_px + tile_px / 2),
                    (int)(points[i].y * tile_px + tile_px / 2),
                    MONSTER_HP);
        monsters[i].glyph = 'm';
        monsters[i].fg_r = 200;
        monsters[i].fg_g = 40;
        monsters[i].fg_b = 40;
        monsters[i].alive = 1;
    }
}

int monsters_get_count(void) {
    return monster_count;
}

monster_t *monsters_get(int i) {
    return &monsters[i];
}

void monsters_render(void) {
    for (int i = 0; i < monster_count; i++) {
        if (!monsters[i].alive) continue;
        monster_t *m = &monsters[i];
        render_draw_glyph_px(m->base.x, m->base.y, m->glyph,
                              m->fg_r, m->fg_g, m->fg_b,
                              0, 0, 0);
    }
}

void monsters_update(const entity_t *player) {
    static int frame_counter = 0;
    frame_counter++;
    if (frame_counter % MONSTER_MOVE_INTERVAL != 0) return;

    unsigned int tile_px = render_get_tile_pitch_px();

    for (int i = 0; i < monster_count; i++) {
        monster_t *m = &monsters[i];
        if (!m->alive) continue;

        int dx = player->x - m->base.x;
        int dy = player->y - m->base.y;

        long dist_sq = (long)dx * dx + (long)dy * dy;
        if (dist_sq > (long)AGGRO_RANGE_PX * AGGRO_RANGE_PX) continue;
        if (dist_sq == 0) continue;

        int step_x = (dx > 0) - (dx < 0);
        int step_y = (dy > 0) - (dy < 0);

        int nx = m->base.x + step_x * m->base.move_speed;
        int ny = m->base.y + step_y * m->base.move_speed;

        int right  = nx + (int)tile_px - 1;
        int bottom = ny + (int)tile_px - 1;
        int blocked =
            dungeon_get_tile(nx    / (int)tile_px, ny     / (int)tile_px) == TILE_WALL ||
            dungeon_get_tile(right / (int)tile_px, ny     / (int)tile_px) == TILE_WALL ||
            dungeon_get_tile(nx    / (int)tile_px, bottom / (int)tile_px) == TILE_WALL ||
            dungeon_get_tile(right / (int)tile_px, bottom / (int)tile_px) == TILE_WALL;

        if (!blocked) {
            m->base.x = nx;
            m->base.y = ny;
        }
    }
}