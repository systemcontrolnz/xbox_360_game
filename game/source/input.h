// input.h — public interface for reading controller input.
// Only file (besides render.c) allowed to call libxenon directly — see
// the platform-abstraction rule in NOTES.md.

#ifndef INPUT_H
#define INPUT_H

// Edge-triggered: true only on the poll where the input transitions from
// released to pressed — not held-down repeat. Matches a turn-based game
// where one press = one action.
typedef struct {
    int up;
    int down;
    int left;
    int right;
    int confirm;   // A button
    int cancel;    // B button
} input_state_t;

// Must be called once at startup, after render_init().
void input_init(void);

// Polls the controller and fills 'out' with newly-pressed inputs since the
// last call. Call once per game loop iteration.
void input_poll(input_state_t *out);

#endif // INPUT_H
