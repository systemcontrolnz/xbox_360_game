#include "render.h"

int main(void) {
    render_init();
    render_clear();
    render_draw_tile(5, 5, '@', 255, 255, 0, 0, 0, 0);
    render_draw_tile(6, 5, 'd', 200, 50, 50, 0, 0, 0);

    while (1) {
    }

    return 0;
}
