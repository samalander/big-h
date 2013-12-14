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


// ===== Defined Constants =====
#define SCREEN_WIDTH 144;
#define SCREEN_HEIGHT 168;

#define WEEKDAY_BG_LAYER_X 0;
#define WEEKDAY_BG_LAYER_Y 0;
#define WEEKDAY_BG_LAYER_WIDTH 15;
#define WEEKDAY_BG_LAYER_HEIGHT SCREEN_HEIGHT;

#define WEEKDAY_LAYER_X 0;
#define WEEKDAY_LAYER_Y 0;
#define WEEKDAY_LAYER_WIDTH 14;
#define WEEKDAY_LAYER_HEIGHT SCREEN_HEIGHT;

#define DATE_BG_LAYER_X 133;
#define DATE_BG_LAYER_Y 0;
#define DATE_BG_LAYER_WIDTH 11;
#define DATE_BG_LAYER_HEIGHT SCREEN_HEIGHT;

#define DATE_LAYER_X 2;
#define DATE_LAYER_Y 0;
#define DATE_LAYER_WIDTH 9;
#define DATE_LAYER_HEIGHT SCREEN_HEIGHT;

#define AMPM_LAYER_X 71;
#define AMPM_LAYER_Y 80;
#define AMPM_LAYER_WIDTH 7;
#define AMPM_LAYER_HEIGHT 12;

#define HOURS_LAYER_X 15;
#define HOURS_LAYER_Y 0;
#define HOURS_LAYER_WIDTH 118;
#define HOURS_LAYER_HEIGHT 80;

#define MINUTES_LAYER_X 15;
#define MINUTES_LAYER_Y 88;
#define MINUTES_LAYER_WIDTH 118;
#define MINUTES_LAYER_HEIGHT 80;

#define SECONDS_BG_LAYER_X 15;
#define SECONDS_BG_LAYER_Y 81;
#define SECONDS_BG_LAYER_WIDTH 118;
#define SECONDS_BG_LAYER_HEIGHT 10;

#define SECONDS_LAYER_X 0;
#define SECONDS_LAYER_Y 4;
#define SECONDS_LAYER_WIDTH 118;
#define SECONDS_LAYER_HEIGHT 2;


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
