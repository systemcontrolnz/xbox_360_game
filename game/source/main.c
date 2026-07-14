#include "render.h"
#include "input.h"
#include "dungeon.h"

int main(void) {
    render_init();
    input_init();
    dungeon_init();

    int x = 5, y = 5;

    render_clear();
    dungeon_render();
    render_draw_tile(x, y, '@', 255, 255, 0, 0, 0, 0);
    render_present();

    while (1) {
        input_state_t in;
        input_poll(&in);

        int nx = x, ny = y;
        if (in.up)    ny--;
        if (in.down)  ny++;
        if (in.left)  nx--;
        if (in.right) nx++;

        int moved = 0;
        if ((nx != x || ny != y) && dungeon_get_tile(nx, ny) != TILE_WALL) {
            x = nx;
            y = ny;
            moved = 1;
        }

        if (moved) {
            render_clear();
            dungeon_render();
            render_draw_tile(x, y, '@', 255, 255, 0, 0, 0, 0);
            render_present();
        }
    }

    return 0;
}
