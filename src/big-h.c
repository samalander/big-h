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
typedef struct Layer_Dim {
  int16_t x, y, width, height;
} Layer_Dim;

static const Layer_Dim Screen_Dim = {0, 0, 144, 168};
static const Layer_Dim Weekday_BG_Layer_Dim = {0, 0, 15, 168};
static const Layer_Dim Weekday_Layer_Dim = {0, 0, 14, 168};
static const Layer_Dim Date_BG_Layer_Dim = {133, 0, 11, 168};
static const Layer_Dim Date_Layer_Dim = {2, 0, 9, 168};
static const Layer_Dim AmPm_Layer_Dim = {71, 80, 7, 12};
static const Layer_Dim Hours_Layer_Dim = {15, 0, 118, 80};
static const Layer_Dim Minutes_Layer_Dim = {15, 88, 118, 80};
static const Layer_Dim Seconds_BG_Layer_Dim = {15, 81, 118, 10};
static const Layer_Dim Seconds_Layer_Dim = {0, 4, 118, 2};


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
