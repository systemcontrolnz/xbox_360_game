// input.c — controller input layer. Only file (besides render.c) allowed
// to call libxenon directly, per NOTES.md's platform-abstraction rule.

#include "input.h"
#include <input/input.h>
#include <usb/usbmain.h>
#include <string.h>

static struct controller_data_s prev;

void input_init(void) {
    usb_init();
    usb_do_poll();
    memset(&prev, 0, sizeof(prev));
}

void input_poll(input_state_t *out) {
    struct controller_data_s cur;
    memset(out, 0, sizeof(*out));

    usb_do_poll();

    if (get_controller_data(&cur, 0)) {
        out->up      = cur.up    && !prev.up;
        out->down    = cur.down  && !prev.down;
        out->left    = cur.left  && !prev.left;
        out->right   = cur.right && !prev.right;
        out->confirm = cur.a     && !prev.a;
        out->cancel  = cur.b     && !prev.b;
        prev = cur;
    }
}
