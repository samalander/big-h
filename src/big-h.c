/*
 * Big H
 * Watchface for Pebble smartwatch
 *
 * https://github.com/samalander/big-h
 *
 * Licensed under the LGPLv3
 * see included LICENSE file or http://www.gnu.org/copyleft/lesser.html
 */

// ===== Included Headers =====
#include <pebble.h>
#include <time.h>

// ===== Global Variables =====
static Window * window;


// initialize, initialize, INITIALIZE!
void handle_init(void) {
    window = window_create();
    window_stack_push(window, true /* Animated */);
}


// destroy, destroy, DESTROY!
void handle_deinit(void) {
    window_destroy(window);
}


// main, main, MAIN?!
int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
