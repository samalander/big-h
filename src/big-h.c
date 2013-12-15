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
typedef struct Rect_Predef {
    int16_t x, y, width, height;
} Rect_Predef;

static const Rect_Predef Screen_Dim = {0, 0, 144, 168};
static const Rect_Predef Weekday_BG_Layer_Dim = {0, 0, 15, 168};
static const Rect_Predef Weekday_Layer_Dim = {0, 0, 14, 168};
static const Rect_Predef Date_BG_Layer_Dim = {133, 0, 11, 168};
static const Rect_Predef Date_Layer_Dim = {2, 0, 9, 168};
static const Rect_Predef AmPm_Layer_Dim = {71, 80, 7, 12};
static const Rect_Predef Hours_Layer_Dim = {15, 0, 118, 80};
static const Rect_Predef Minutes_Layer_Dim = {15, 88, 118, 80};
static const Rect_Predef Seconds_BG_Layer_Dim = {15, 81, 118, 10};
static const Rect_Predef Seconds_Layer_Dim = {0, 4, 118, 2};
static const Rect_Predef Time_Digit[2] = {{0, 0, 60, 82}, {60, 0, 60, 82}};

static const int16_t Offset = 1;

static const int16_t Am_Base_Y = 3,
                     Pm_Base_Y = 8,
                     AmPm_Width = 6,
                     AmPm_Height = 4;

static const int16_t Seconds_BG_Y1 = 3,
                     Seconds_BG_Y2 = 6,
                     Seconds_Width = 2,
                     Seconds_Height = 2,
                     Seconds_BG_Ind30_X = 59,
                     Seconds_BG_Ind15_X = 29,
                     Seconds_BG_Ind45_X = 89;

static int16_t mday_max[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


// ===== Global Variables =====
static Window * window;
static Layer * window_layer,
             * weekday_bg_layer,
             * weekday_layer,
             * date_bg_layer,
             * date_layer,
             * ampm_layer,
             * hours_layer,
             * minutes_layer,
             * seconds_bg_layer,
             * seconds_layer;
GBitmap * weekday_digits[10],
        * weekday_names[7],
        * date_digits[10],
        * time_digits[10];
struct tm current_time;


// Drawing the weekday background layer
static void weekday_bg_layer_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(bounds.size.w - Offset, 0), GPoint(bounds.size.w - Offset, bounds.size.h - Offset));
}


// Drawing the date background layer
static void date_bg_layer_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, bounds.size.h - Offset));
}


// Drawing the seconds background layer
static void seconds_bg_layer_draw(Layer *layer, GContext *ctx) {
    // Horizontal container lines
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(0, Seconds_BG_Y1), GPoint(bounds.size.w - Offset, Seconds_BG_Y1));
    graphics_draw_line(ctx, GPoint(0, Seconds_BG_Y2), GPoint(bounds.size.w - Offset, Seconds_BG_Y2));

    // Halfway indicator (30s)
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind30_X, 0), GPoint(Seconds_BG_Ind30_X, Offset));
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind30_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind30_X, Seconds_BG_Y2 + (Offset * 2) + Offset));

    // Quarter indicator (15s)
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind15_X, Offset), GPoint(Seconds_BG_Ind15_X, Offset));
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind15_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind15_X, Seconds_BG_Y2 + (Offset * 2)));

    // Three-quarter indicator (45s)
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind45_X, Offset), GPoint(Seconds_BG_Ind45_X, Offset));
    graphics_draw_line(ctx, GPoint(Seconds_BG_Ind45_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind45_X, Seconds_BG_Y2 + (Offset * 2)));
}


// Drawing the weekday layer
static void weekday_layer_draw(Layer *layer, GContext *ctx) {

}


// Drawing the date layer
static void date_layer_draw(Layer *layer, GContext *ctx) {
    
}


// Drawing the ampm layer
static void ampm_layer_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    int16_t step = current_time.tm_hour < 12 ? -Offset : Offset,
            base_y = current_time.tm_hour < 12 ? Am_Base_Y : Pm_Base_Y;
    for (int16_t i = 0; i < AmPm_Height; i++) {
        graphics_draw_line(ctx, GPoint(0 + i, base_y + (i * step)), GPoint(AmPm_Width - i, base_y + (i * step)));
    }
}


// Drawing the hours layer
static void hours_layer_draw(Layer *layer, GContext *ctx) {
    int16_t hour = current_time.tm_hour;
    // Deal with the 12-hour clock
    if (!clock_is_24h_style()) {
        if (hour > 12) {
            hour = hour - 12;
        }
        if (hour == 0) {
            hour = 12;
        }
    }
    if (hour >= 10) {
        graphics_draw_bitmap_in_rect(ctx, time_digits[hour / 10], GRect(Time_Digit[0].x, Time_Digit[0].y, Time_Digit[0].width, Time_Digit[0].height));
    }
    graphics_draw_bitmap_in_rect(ctx, time_digits[hour % 10], GRect(Time_Digit[1].x, Time_Digit[1].y, Time_Digit[1].width, Time_Digit[1].height));
}


// Drawing the minutes layer
static void minutes_layer_draw(Layer *layer, GContext *ctx) {
    graphics_draw_bitmap_in_rect(ctx, time_digits[current_time.tm_min / 10], GRect(Time_Digit[0].x, Time_Digit[0].y, Time_Digit[0].width, Time_Digit[0].height));
    graphics_draw_bitmap_in_rect(ctx, time_digits[current_time.tm_min % 10], GRect(Time_Digit[1].x, Time_Digit[1].y, Time_Digit[1].width, Time_Digit[1].height));
}


// Drawing the seconds layer
static void seconds_layer_draw(Layer *layer, GContext *ctx) {
    // Draw nothing if we're at zero
    if (current_time.tm_sec > 0) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_draw_rect(ctx, GRect(0, 0, current_time.tm_sec * Seconds_Width, Seconds_Height));
    }
}


// Handle the tick event
static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    // Populate the global variable with the current time to have it when painting
    current_time = *tick_time;

    // Mark the relevant layers for refresh
    layer_mark_dirty(seconds_layer);
    if (current_time.tm_sec == 0) {
        layer_mark_dirty(minutes_layer);
        if (current_time.tm_min == 0) {
            layer_mark_dirty(hours_layer);
            if (!clock_is_24h_style() && current_time.tm_hour % 12 == 0) {
                layer_mark_dirty(ampm_layer);
            }
            if (current_time.tm_hour == 0) {
                layer_mark_dirty(weekday_layer);
                layer_mark_dirty(date_layer);
            }
        }
    }
}


// initialize, initialize, INITIALIZE!
void handle_init(void) {
    // Populate the global time variable so we have it when painting
    time_t timer = time(NULL);
    current_time = *localtime(&timer);

    // Setting up our window
    window = window_create();
    window_stack_push(window, true /* Animated */);
    window_set_background_color(window, GColorBlack);
    window_layer = window_get_root_layer(window);

    // Layers
    weekday_bg_layer = layer_create(GRect(Weekday_BG_Layer_Dim.x, Weekday_BG_Layer_Dim.y, Weekday_BG_Layer_Dim.width, Weekday_BG_Layer_Dim.height));
    layer_set_update_proc(weekday_bg_layer, weekday_bg_layer_draw);
    layer_add_child(window_layer, weekday_bg_layer);

    date_bg_layer = layer_create(GRect(Date_BG_Layer_Dim.x, Date_BG_Layer_Dim.y, Date_BG_Layer_Dim.width, Date_BG_Layer_Dim.height));
    layer_set_update_proc(date_bg_layer, date_bg_layer_draw);
    layer_add_child(window_layer, date_bg_layer);

    weekday_layer = layer_create(GRect(Weekday_Layer_Dim.x, Weekday_Layer_Dim.y, Weekday_Layer_Dim.width, Weekday_Layer_Dim.height));
    layer_set_update_proc(weekday_layer, weekday_layer_draw);
    layer_add_child(weekday_bg_layer, weekday_layer);

    date_layer = layer_create(GRect(Date_Layer_Dim.x, Date_Layer_Dim.y, Date_Layer_Dim.width, Date_Layer_Dim.height));
    layer_set_update_proc(date_layer, date_layer_draw);
    layer_add_child(date_bg_layer, date_layer);

    hours_layer = layer_create(GRect(Hours_Layer_Dim.x, Hours_Layer_Dim.y, Hours_Layer_Dim.width, Hours_Layer_Dim.height));
    layer_set_update_proc(hours_layer, hours_layer_draw);
    layer_add_child(window_layer, hours_layer);

    minutes_layer = layer_create(GRect(Minutes_Layer_Dim.x, Minutes_Layer_Dim.y, Minutes_Layer_Dim.width, Minutes_Layer_Dim.height));
    layer_set_update_proc(minutes_layer, minutes_layer_draw);
    layer_add_child(window_layer, minutes_layer);

    ampm_layer = layer_create(GRect(AmPm_Layer_Dim.x, AmPm_Layer_Dim.y, AmPm_Layer_Dim.width, AmPm_Layer_Dim.height));
    layer_set_update_proc(ampm_layer, ampm_layer_draw);
    layer_add_child(window_layer, ampm_layer);

    seconds_bg_layer = layer_create(GRect(Seconds_BG_Layer_Dim.x, Seconds_BG_Layer_Dim.y, Seconds_BG_Layer_Dim.width, Seconds_BG_Layer_Dim.height));
    layer_set_update_proc(seconds_bg_layer, seconds_bg_layer_draw);
    layer_add_child(window_layer, seconds_bg_layer);

    seconds_layer = layer_create(GRect(Seconds_Layer_Dim.x, Seconds_Layer_Dim.y, Seconds_Layer_Dim.width, Seconds_Layer_Dim.height));
    layer_set_update_proc(seconds_layer, seconds_layer_draw);
    layer_add_child(seconds_bg_layer, seconds_layer);

    // Bitmap resources
    time_digits[0] = gbitmap_create_with_resource(RESOURCE_ID_TIME_0);
    time_digits[1] = gbitmap_create_with_resource(RESOURCE_ID_TIME_1);
    time_digits[2] = gbitmap_create_with_resource(RESOURCE_ID_TIME_2);
    time_digits[3] = gbitmap_create_with_resource(RESOURCE_ID_TIME_3);
    time_digits[4] = gbitmap_create_with_resource(RESOURCE_ID_TIME_4);
    time_digits[5] = gbitmap_create_with_resource(RESOURCE_ID_TIME_5);
    time_digits[6] = gbitmap_create_with_resource(RESOURCE_ID_TIME_6);
    time_digits[7] = gbitmap_create_with_resource(RESOURCE_ID_TIME_7);
    time_digits[8] = gbitmap_create_with_resource(RESOURCE_ID_TIME_8);
    time_digits[9] = gbitmap_create_with_resource(RESOURCE_ID_TIME_9);


    // Subscribing to the tick event
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}


// destroy, destroy, DESTROY!
void handle_deinit(void) {
    // Bitmap resources
    for (int16_t i = 0; i < 10; i++) {
        gbitmap_destroy(time_digits[i]);
    }

    // Display layers
    layer_destroy(weekday_layer);
    layer_destroy(date_layer);
    layer_destroy(ampm_layer);
    layer_destroy(hours_layer);
    layer_destroy(minutes_layer);
    layer_destroy(seconds_layer);

    // Background layers
    layer_destroy(weekday_bg_layer);
    layer_destroy(date_bg_layer);
    layer_destroy(seconds_bg_layer);

    window_destroy(window);
}


// main, main, MAIN?!
int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
