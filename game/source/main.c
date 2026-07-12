#include "render.h"
#include "input.h"

int main(void) {
    render_init();
    input_init();

    int x = 5, y = 5;

    render_clear();
    render_draw_tile(x, y, '@', 255, 255, 0, 0, 0, 0);

    while (1) {
        input_state_t in;
        input_poll(&in);

        int moved = 0;
        if (in.up)    { y--; moved = 1; }
        if (in.down)  { y++; moved = 1; }
        if (in.left)  { x--; moved = 1; }
        if (in.right) { x++; moved = 1; }

        if (moved) {
            render_clear();
            render_draw_tile(x, y, '@', 255, 255, 0, 0, 0, 0);
        }
    }

    return 0;
}
