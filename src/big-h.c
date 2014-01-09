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


// ===== Function Declarations =====
void handle_init(void);
void handle_deinit(void);

// ===== Defined Constants =====
typedef struct Rect_Predef {
    int16_t x, y, width, height;
} Rect_Predef;

enum weekday_format_types {
    INTL = 0,
    EN = 1,
    FR = 2,
    ES = 3
};

enum settings_app_message_keys {
    Weekday_Format = 0,
    Vibrate_on_Hour = 1,
    Display_Seconds = 2,
    Leading_Zero = 3,
    Weekday_First_Day = 4,
    Date_Format = 5,
    Display_Battery = 6
};

static const int16_t Settings_Storage_Key = 0;

typedef struct Settings_Type {
    enum weekday_format_types weekday_format;
    bool vibrate_on_hour;
    bool display_seconds;
    bool leading_zero;
    int16_t weekday_first_day;
    char date_format[10];
    bool display_battery;
} __attribute__((__packed__)) Settings_Type;

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
static const Rect_Predef Date_Digit = {0, 0, 10, 13};

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

static const int16_t Date_Max_Char = 10,
                     Date_Char_Height = 16,
                     Date_Char_Space = 5;

static const int16_t Weekday_Day_Height = 24,
                     Weekday_Nb = 7,
                     Weekday_Max_Delta = 3;

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
int16_t battery_state;
struct Settings_Type settings;


// Drawing the weekday background layer
static void weekday_bg_layer_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(bounds.size.w - Offset, 0), GPoint(bounds.size.w - Offset, bounds.size.h - Offset));

    if (settings.weekday_format == INTL) {
        for (int16_t i = 1; i < Weekday_Nb; i++) {
            graphics_draw_line(ctx, GPoint(0, i * Weekday_Day_Height), GPoint(bounds.size.w, i * Weekday_Day_Height));
        }
    }
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
    int16_t seconds_bg_y_offset = settings.display_seconds ? 0 : 1;
    
    graphics_draw_line(ctx, GPoint(0, Seconds_BG_Y1 + seconds_bg_y_offset), GPoint(bounds.size.w * battery_state / 100, Seconds_BG_Y1 + seconds_bg_y_offset));
    graphics_draw_line(ctx, GPoint(0, Seconds_BG_Y2 - seconds_bg_y_offset), GPoint(bounds.size.w * battery_state / 100, Seconds_BG_Y2 - seconds_bg_y_offset));

    if (settings.display_seconds || settings.display_battery) {
        // Halfway indicator (30s / 50%)
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind30_X, 0), GPoint(Seconds_BG_Ind30_X, Offset));
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind30_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind30_X, Seconds_BG_Y2 + (Offset * 2) + Offset));

        // Quarter indicator (15s / 25%)
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind15_X, Offset), GPoint(Seconds_BG_Ind15_X, Offset));
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind15_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind15_X, Seconds_BG_Y2 + (Offset * 2)));

        // Three-quarter indicator (45s / 75%)
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind45_X, Offset), GPoint(Seconds_BG_Ind45_X, Offset));
        graphics_draw_line(ctx, GPoint(Seconds_BG_Ind45_X, Seconds_BG_Y2 + (Offset * 2)), GPoint(Seconds_BG_Ind45_X, Seconds_BG_Y2 + (Offset * 2)));
    }
}


// Drawing the weekday layer
static void weekday_layer_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);

    if (settings.weekday_format == INTL) {
        for (int16_t position = 0; position < Weekday_Nb; position++) {
            // Day of the month being drawn, based on the current day
            int16_t mday = current_time.tm_mday;
            // Day of the week being drawn, based on the current weekday
            int16_t wday = current_time.tm_wday;
            // Handle displaying a day other than Sunday as the first day
            wday = (wday + (Weekday_Nb - settings.weekday_first_day)) % Weekday_Nb;
            // Difference between the position we're drawing and the current weekday
            int16_t mday_delta = position - wday;

            if (mday_delta < -Weekday_Max_Delta) {
                // If the day delta is too low, we actually draw a day in the future instead  (Sunday to Tuesday when currently Thursday or later in the week)
                mday_delta += Weekday_Nb;
            }
            else if (mday_delta > Weekday_Max_Delta) {
                // If the day delta is too high, we actually draw a day in the past instead (Thursday to Saturday when currently Tuesday or earlier in the week)
                mday_delta -= Weekday_Nb;
            }

            // To separate the weekdays in the past from those in the future, we draw an arrow on the border (except if that border is spread out on the screen edges)
            int16_t line;
            switch (mday_delta) {
                // Draw the bottom part of the arrow
                case -3:
                    if (position > 0) {
                        line = position * 24;
                        for (int i = 2; i < 7; i += 2) {
                            graphics_draw_line(ctx, GPoint(i, ++line), GPoint(13 - i, line));
                        }
                    }
                    break;
                // Draw the top part of the arrow
                case 3:
                    if (position < 6) {
                        line = (position + 1) * 24;
                        for (int i = 0; i < 6; i += 2) {
                            graphics_draw_line(ctx, GPoint(0, --line), GPoint(5 - i, line));
                            graphics_draw_line(ctx, GPoint(8 + i, line), GPoint(13, line));
                        }
                    }
                    break;
            }

            // Decide what the day of the month we're drawing actually is
            mday += mday_delta;
            if (mday < 1) {
                // If we're in the past and accross a month's start, get the last day of the previous month and count from there
                int16_t new_month = current_time.tm_mon - 1;
                // watch out for December...
                if (new_month == -1) {
                    new_month = 11;
                }
                mday += mday_max[new_month];
                // ... and February on leap years (not checking for the 100s and 400s rules since they won't occur for 87 years...)
                if ((new_month == 1) && ((current_time.tm_year + 1900) % 4 == 0)) {
                    mday++;
                }
            }
            else if (mday > mday_max[current_time.tm_mon]) {
                // If we're in the future and past the current month's end, start from 1 in the next month
                int mday_mod = mday_max[current_time.tm_mon];
                // watch out for February in leap years here too
                if ((current_time.tm_mon == 1) && ((current_time.tm_year + 1900) % 4 == 0)) {
                    mday_mod++;
                }
                mday %= mday_mod;
            }

            // Decide where to draw and in what color
            int base_y = (position * Weekday_Day_Height) + 12 - 5;
            int color = (bool)(wday == position);
            if (color) {
                // If we're drawing today's date, we need to fill in the background in white...
                graphics_context_set_fill_color(ctx, GColorWhite);
                graphics_fill_rect(ctx, GRect(0, position * Weekday_Day_Height, 14, Weekday_Day_Height), 0, GCornerNone);
                // ... and draw our digits in inverted mode
                graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
            }
            // draw the actual digits from what we decided above
            if (mday >= 10) {
                graphics_draw_bitmap_in_rect(ctx, weekday_digits[mday / 10], GRect(0, base_y, 6, 11));
            }
            graphics_draw_bitmap_in_rect(ctx, weekday_digits[mday % 10], GRect(7, base_y, 6, 11));
            if (color) {
                // Switching the context back to normal mode
                graphics_context_set_compositing_mode(ctx, GCompOpAssign);
            }
        }
    }
    else {
        graphics_draw_bitmap_in_rect(ctx, weekday_names[current_time.tm_wday], GRect(Weekday_Layer_Dim.x, Weekday_Layer_Dim.y, Weekday_Layer_Dim.width, Weekday_Layer_Dim.height));
    }
}


// Drawing the date layer
static void date_layer_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);

    // Fill the date array with our date data
    int16_t year = current_time.tm_year + 1900;
    int16_t month = current_time.tm_mon + 1;

    int16_t date_int[40] = { [0 ... 9] = INT16_MIN};
    int16_t format_length;

    format_length = strlen(settings.date_format);

    int16_t j = 0;
    for (int16_t i = 0; i < format_length; i++) {
        switch (settings.date_format[i]) {
            case '-':
                date_int[j++] = -1;
                break;
            case '/':
                date_int[j++] = -2;
                break;
            case '.':
                date_int[j++] = -3;
                break;
            case ' ':
                j++;
                break;
            case 'Y':
                date_int[j] = year / 1000;
                date_int[++j] = year % 1000 / 100;
                date_int[++j] = year % 100 / 10;
                date_int[++j] = year % 10;
                j++;
                break;
            case 'y':
                date_int[j] = year % 100 / 10;
                date_int[++j] = year % 10;
                j++;
                break;
            case 'M':
                date_int[j] = month / 10;
                date_int[++j] = month % 10;
                j++;
                break;
            case 'm':
                if (month >= 10) {
                    date_int[j] = month / 10;
                    date_int[++j] = month % 10;
                }
                else {
                    date_int[j] = month % 10;
                }
                j++;
                break;
            case 'D':
                date_int[j] = current_time.tm_mday / 10;
                date_int[++j] = current_time.tm_mday % 10;
                j++;
                break;
            case 'd':
                if (current_time.tm_mday >= 10) {
                    date_int[j] = current_time.tm_mday / 10;
                    date_int[++j] = current_time.tm_mday % 10;
                }
                else {
                    date_int[j] = current_time.tm_mday % 10;
                }
                j++;
                break;
            // Any other character has no meaning so we ignore it
        }
    }
    
    // Make sure we won't run off the screen
    j = j <= 10 ? j : 10;

    int16_t base_y;
    int16_t offset_y = ((Date_Max_Char - j) / 2 * Date_Char_Height) + ((j % 2) * (Date_Char_Height / 2));

    // Loop over the array to draw the digits
    for (int16_t i = 0; i < j; i++) {
        base_y = offset_y + (i * Date_Char_Height) + Date_Char_Space;
        if (date_int[i] == -1) {
            graphics_draw_line(ctx, GPoint(2, base_y + 7), GPoint(6, base_y + 7));
        }
        else if (date_int[i] == -2) {
            graphics_draw_line(ctx, GPoint(1, base_y + 13), GPoint(8, base_y));
        }
        else if (date_int[i] == -3) {
            graphics_context_set_fill_color(ctx, GColorWhite);
            graphics_fill_circle(ctx, GPoint(4, base_y + 7), 1);
        }
        else if (date_int[i] >= 0 && date_int[i] <= 9) {
            graphics_draw_bitmap_in_rect(ctx, date_digits[date_int[i]], GRect(Date_Digit.x, base_y, Date_Digit.width, Date_Digit.height));
        }
        // Anything else will leave a blank space
    }
}


// Drawing the ampm layer
static void ampm_layer_draw(Layer *layer, GContext *ctx) {
    if (!clock_is_24h_style()) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        int16_t step = current_time.tm_hour < 12 ? -Offset : Offset,
                base_y = current_time.tm_hour < 12 ? Am_Base_Y : Pm_Base_Y;
        for (int16_t i = 0; i < AmPm_Height; i++) {
            graphics_draw_line(ctx, GPoint(0 + i, base_y + (i * step)), GPoint(AmPm_Width - i, base_y + (i * step)));
        }
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
    if (settings.leading_zero || hour >= 10) {
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
    if (settings.display_seconds) {
        layer_mark_dirty(seconds_layer);
    }
    if (current_time.tm_sec == 0) {
        layer_mark_dirty(minutes_layer);
        if (current_time.tm_min == 0) {
            layer_mark_dirty(hours_layer);
            if (settings.vibrate_on_hour) {
                vibes_short_pulse();
            }
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

// Handle the battery change event
static void handle_battery(BatteryChargeState charge_state) {
    battery_state = charge_state.charge_percent;
    layer_mark_dirty(seconds_bg_layer);
}


// Outgoing message was delivered
void out_sent_handler(DictionaryIterator *sent, void *context) {

}


// Outgoing message failed
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {

}


// Incoming message received
void in_received_handler(DictionaryIterator *received, void *context) {
    bool settingsUpdated = false;

    Tuple *tuple = dict_read_first(received);
    while (tuple) {
        switch (tuple->key) {
            case Weekday_Format:
                settings.weekday_format = (enum weekday_format_types)tuple->value->data;
                settingsUpdated = true;
                break;
            case Vibrate_on_Hour:
                settings.vibrate_on_hour = (bool)tuple->value->data;
                settingsUpdated = true;
                break;
            case Display_Seconds:
                settings.display_seconds = (bool)tuple->value->data;
                settingsUpdated = true;
                break;
            case Leading_Zero:
                settings.leading_zero = (bool)tuple->value->data;
                settingsUpdated = true;
                break;
            case Weekday_First_Day:
                settings.weekday_first_day = (int32_t)tuple->value->data;
                settingsUpdated = true;
                break;
            case Date_Format:
                strncpy(settings.date_format, tuple->value->cstring, 10);
                settingsUpdated = true;
                break;
            case Display_Battery:
                settings.display_battery = (bool)tuple->value->data;
                settingsUpdated = true;
                break;
        }
        tuple = dict_read_next(received);
    }

    if (settingsUpdated) {
        persist_write_data(Settings_Storage_Key, &settings, sizeof(settings));
    }
}


// Incoming message dropped
void in_dropped_handler(AppMessageResult reason, void *context) {

}


// Get our settings from the phone, locally or use the defaults
void init_settings(void) {
    // Check local storage for our settings
    if (persist_exists(Settings_Storage_Key)) {
        // If settings exist locally, load them
        persist_read_data(Settings_Storage_Key, &settings, sizeof(settings));
    }
    else {
        // Otherwise, use the defaults
        settings.weekday_format = INTL;
        settings.vibrate_on_hour = false;
        settings.display_seconds = true;
        settings.leading_zero = false;
        settings.weekday_first_day = 0;
        if (!clock_is_24h_style()) {
            strncpy(settings.date_format, "M/D/Y", 10);
        }
        else {
            strncpy(settings.date_format, "Y-M-D", 10);
        }
        settings.display_battery = true;
    }
}


// initialize, initialize, INITIALIZE!
void handle_init(void) {
    // Populate the global time and battery variables so we have them when painting
    init_settings();
    time_t timer = time(NULL);
    current_time = *localtime(&timer);
    if (settings.display_battery) {
        BatteryChargeState battery_peek = battery_state_service_peek();
        battery_state = battery_peek.charge_percent;
    }
    else {
        battery_state = 100;
    }

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

    date_digits[0] = gbitmap_create_with_resource(RESOURCE_ID_DATE_0);
    date_digits[1] = gbitmap_create_with_resource(RESOURCE_ID_DATE_1);
    date_digits[2] = gbitmap_create_with_resource(RESOURCE_ID_DATE_2);
    date_digits[3] = gbitmap_create_with_resource(RESOURCE_ID_DATE_3);
    date_digits[4] = gbitmap_create_with_resource(RESOURCE_ID_DATE_4);
    date_digits[5] = gbitmap_create_with_resource(RESOURCE_ID_DATE_5);
    date_digits[6] = gbitmap_create_with_resource(RESOURCE_ID_DATE_6);
    date_digits[7] = gbitmap_create_with_resource(RESOURCE_ID_DATE_7);
    date_digits[8] = gbitmap_create_with_resource(RESOURCE_ID_DATE_8);
    date_digits[9] = gbitmap_create_with_resource(RESOURCE_ID_DATE_9);

    switch (settings.weekday_format) {
        case INTL:
            weekday_digits[0] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_0);
            weekday_digits[1] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_1);
            weekday_digits[2] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_2);
            weekday_digits[3] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_3);
            weekday_digits[4] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_4);
            weekday_digits[5] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_5);
            weekday_digits[6] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_6);
            weekday_digits[7] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_7);
            weekday_digits[8] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_8);
            weekday_digits[9] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_9);
            break;
        case EN:
            weekday_names[0] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_0);
            weekday_names[1] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_1);
            weekday_names[2] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_2);
            weekday_names[3] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_3);
            weekday_names[4] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_4);
            weekday_names[5] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_5);
            weekday_names[6] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_EN_6);
            break;
        case FR:
            weekday_names[0] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_0);
            weekday_names[1] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_1);
            weekday_names[2] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_2);
            weekday_names[3] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_3);
            weekday_names[4] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_4);
            weekday_names[5] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_5);
            weekday_names[6] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_FR_6);
            break;
        case ES:
            weekday_names[0] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_0);
            weekday_names[1] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_1);
            weekday_names[2] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_2);
            weekday_names[3] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_3);
            weekday_names[4] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_4);
            weekday_names[5] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_5);
            weekday_names[6] = gbitmap_create_with_resource(RESOURCE_ID_WEEKDAY_ES_6);
            break;
    }

    // Subscribing to the tick event
    tick_timer_service_subscribe(settings.display_seconds ? SECOND_UNIT: MINUTE_UNIT, handle_tick);

    if (settings.display_battery) {
        // Subscribing to the battery change event
        battery_state_service_subscribe(handle_battery);
    }

    // Registering the messaging handlers
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);

    const uint32_t inbound_size = 64;
    const uint32_t outbound_size = 64;
    app_message_open(inbound_size, outbound_size);
}


// destroy, destroy, DESTROY!
void handle_deinit(void) {
    // Bitmap resources
    for (int16_t i = 0; i < 10; i++) {
        gbitmap_destroy(time_digits[i]);
        gbitmap_destroy(date_digits[i]);
        if (settings.weekday_format == INTL) {
            gbitmap_destroy(weekday_digits[i]);
        }
    }
    if (settings.weekday_format != INTL) {
        for (int16_t i = 0; i < 7; i++) {
            gbitmap_destroy(weekday_names[i]);
        }
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
