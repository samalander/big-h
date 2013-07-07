/*
 * Big H
 * Watchface for Pebble smartwatch
 *
 * https://github.com/samalander/big-h
 *
 * Licensed under the LGPLv3
 * see included LICENSE file or http://www.gnu.org/copyleft/lesser.html
 */

#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID { 0xF0, 0x2A, 0xFD, 0x8F, 0x3A, 0xEC, 0x4A, 0x43, 0x96, 0x45, 0x7A, 0x4B, 0x62, 0x8F, 0x29, 0xB5 }
PBL_APP_INFO(MY_UUID,
             "Big H", "Samalander",
             1, 1, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define WEEKDAY_FORMAT_INTL 0
// [[EN]]
#define WEEKDAY_FORMAT_EN 1
// [[FR]]
#define WEEKDAY_FORMAT_FR 2
// [[ES]]
#define WEEKDAY_FORMAT_ES 3
#define DATE_FORMAT_AUTO "auto"

/************** SETTINGS - START **************/

// [[V]]
// set to true if you'd like your pebble to vibrate on the hour
#define VIBRATE_ON_HOUR false

// [[NS]]
// set to false to disable the seconds display
#define DISPLAY_SECONDS true

// [[H0]]
// set to true to display the leading zero on the hour
#define HOUR_ZERO false

// [[M1]]
// set to true to make Monday the first day of the week in the international weekday display (no effect on other weekday displays)
#define WEEKDAY_MONDAY_FIRST false

// see variables above to select a language for the weekday display
#define WEEKDAY_FORMAT WEEKDAY_FORMAT_INTL

/*
 * By default, the date will be YYYY-MM-DD if the clock is 24-hour style and MM-DD-YYYY if the clock is 12-hour style.
 *
 * [[MDY]]
 * Change to "M/D/Y" if you'd like to always use US-style date format MM/DD/YYYY.
 * [[YMD]]
 * Change to "Y-M-D" if you'd like to always use the YYYY-MM-DD format.
 * [[DMY]]
 * Change to "D-M-Y" if you'd like to always use the DD-MM-YYYY format.
 *
 * Any other order of 'Y', 'M' and 'D' will also work.  You can even omit those you don't want.
 * Lowercase 'y', 'm' or 'd' may also be used.  'y' will use a 2-digit year, 'm' and 'd' will omit any leading zeroes.
 * Separators can be either '-', '/', '.' or ' ' at your choice.
 * Any other character will be ignored.
 */
#define DATE_FORMAT DATE_FORMAT_AUTO

/************** SETTINGS - END **************/

Window window;
Layer weekday_background_layer, weekday_text_layer, date_background_layer, date_text_layer, hours_layer, minutes_layer, seconds_background_layer, seconds_indicator_layer, ampm_layer;
BmpContainer date_digits[10], time_digits[10];
#if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
  BmpContainer weekday_digits[2][10];
#else
  BmpContainer weekday_name[7];
#endif
static int mday_max[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Global time to have access to it in draw events
PblTm tick_time;


// draw the static background of the weekday column on the left of the screen
void paint_weekday_background(Layer *layer, GContext *ctx) {
  // draw the vertical line on the right
  graphics_draw_line(ctx, GPoint(14, 0), GPoint(14, 167));

  #if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
    //draw the horizontal lines that separate the 7 weekday slots
    graphics_draw_line(ctx, GPoint(0, 24), GPoint(13, 24));
    graphics_draw_line(ctx, GPoint(0, 48), GPoint(13, 48));
    graphics_draw_line(ctx, GPoint(0, 72), GPoint(13, 72));
    graphics_draw_line(ctx, GPoint(0, 96), GPoint(13, 96));
    graphics_draw_line(ctx, GPoint(0, 120), GPoint(13, 120));
    graphics_draw_line(ctx, GPoint(0, 144), GPoint(13, 144));
  #endif
}

#if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
  // draw the content of one slot in the weekday column
  void paint_weekday_text_single(GContext *ctx, int position) {
    // day of the month being drawn, based on the current day
    int mday = tick_time.tm_mday;
    // day of the week being drawn, base on the current weekday
    int wday = tick_time.tm_wday;
    #if WEEKDAY_MONDAY_FIRST
      wday = (wday + 6) % 7;
    #endif
    // difference between the position we're drawing and the current weekday
    int mday_delta = position - wday;

    if (mday_delta < -3) {
      // if the day delta is too low, we actually draw a day in the future instead (Sunday to Tuesday when currently Thursday or later in the week)
      mday_delta += 7;
    }
    else if (mday_delta > 3) {
      // if the day delta is too high, we actually draw a day in the past instead (Thursday to Saturday when currently Tuesday or earlier in the week)
      mday_delta -= 7;
    }


    // to separate the weekdays in the past from those in the future, we draw an arrow on the border (except if that border is spread out on the screen edges)
    int line;
    switch (mday_delta) {
      // draw the bottom part of the arrow
      case -3:
        if (position > 0) {
          line = position * 24;
          for (int i = 2; i < 7; i += 2) {
            graphics_draw_line(ctx, GPoint(i, ++line), GPoint(13 - i, line));
          }
        }
        break;
      // draw the top part of the arrow
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

    // decide what the day of the month we're drawing actually is
    mday += mday_delta;
    if (mday < 1) {
      // if we're in the past and accross a month's start, get the last day of the previous month and count from there
      int new_month = tick_time.tm_mon - 1;
      // watch out for December...
      if (new_month == -1) {
        new_month = 11;
      }
      mday += mday_max[new_month];
      // ... and February on leap years (not checking for the 100s and 400s rules since they won't occur for 87 years...)
      if ((new_month == 1) && ((tick_time.tm_year + 1900) % 4 == 0)) {
        mday++;
      }
    }
    else if (mday > mday_max[tick_time.tm_mon]) {
      // if we're in the future and past the current month's end, start from 1 in the next month
      int mday_mod = mday_max[tick_time.tm_mon];
      // watch out for February in leap years here too
      if ((tick_time.tm_mon == 1) && ((tick_time.tm_year + 1900) % 4 == 0)) {
        mday_mod++;
      }
      mday %= mday_mod;
    }
    // decide where to draw and in what color
    int base_y = (position * 24) + 12 - 5;
    int color = (bool)(wday == position);
    if (color) {
      // if we're drawing today's date, we need to fill in the background in white
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(0, position * 24, 14, 24), 0, GCornerNone);
    }
    // draw the actual digits from what we decided above
    if (mday >= 10) {
      graphics_draw_bitmap_in_rect(ctx, &weekday_digits[color][mday / 10].bmp, GRect(0, base_y, 6, 11));
    }
    graphics_draw_bitmap_in_rect(ctx, &weekday_digits[color][mday % 10].bmp, GRect(7, base_y, 6, 11));
  }
#endif

// draw the weekdays in all 7 slots
void paint_weekday_text(Layer *layer, GContext *ctx) {
  #if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
    for (int i = 0; i < 7; i++) {
      paint_weekday_text_single(ctx, i);
    }
  #else
    graphics_draw_bitmap_in_rect(ctx, &weekday_name[tick_time.tm_wday].bmp, GRect(0, 0, 14, 168));
  #endif
}


// draw the static background on the right of the screen
void paint_date_background(Layer *layer, GContext *ctx) {
  graphics_draw_line(ctx, GPoint(0, 0), GPoint(0, 167));
}


// draw all the digits of the date from the array we're passed
void paint_date_digits(GContext *ctx, int *digits, int length) {
  int base_y;
  int offset = (10 - length) / 2 * 16;
  // loop over the array to draw the digits
  for (int i = 0; i < 10; i++) {
    base_y = offset + (i * 16) + 5;
    if (digits[i] == -1) {
      // we're drawing a dash for unit separation
      graphics_draw_line(ctx, GPoint(2, base_y + 7), GPoint(6, base_y + 7));
    }
    else if (digits[i] == -2) {
      // we're drawing a slash for unit separation
      graphics_draw_line(ctx, GPoint(1, base_y + 13), GPoint(8, base_y));
    }
    else if (digits[i] == -3) {
      // we're drawing a dot for unit separation
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_circle(ctx, GPoint(4, base_y + 7), 1);
    }
    else if (digits[i] >= 0 && digits[i] <= 9) {
      // actually drawing a digit!
      graphics_draw_bitmap_in_rect(ctx, &date_digits[digits[i]].bmp, GRect(0, base_y, 10, 13));
    }
    // Anything else will leave a blank space
  }
}


// draw the date
void paint_date_text(Layer *layer, GContext *ctx) {
  // fill the date array with our date data
  int year = tick_time.tm_year + 1900;
  int month = tick_time.tm_mon + 1;

  char date_str[10];
  int date_int[10] = { [0 ... 9] = 255 };
  if (strcmp(DATE_FORMAT, DATE_FORMAT_AUTO) == 0) {
    if (!clock_is_24h_style()) {
      strcpy(date_str, "M/D/Y");
    }
    else {
      strcpy(date_str, "Y-M-D");
    }
  }
  else {
    strcpy(date_str, DATE_FORMAT);
  }

  int j = 0;
  for (int i = 0; i < 10; i++) {
    switch (date_str[i]) {
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
        date_int[j] = tick_time.tm_mday / 10;
        date_int[++j] = tick_time.tm_mday % 10;
        j++;
        break;
      case 'd':
        if (tick_time.tm_mday >= 10) {
          date_int[j] = tick_time.tm_mday / 10;
          date_int[++j] = tick_time.tm_mday % 10;
        }
        else {
          date_int[j] = tick_time.tm_mday % 10;
        }
        j++;
        break;
   }
  }

  // draw the actually digits now that we know where to put them
  paint_date_digits(ctx, date_int, j);
}


// draw the hour
void paint_hours(Layer *layer, GContext *ctx) {
  int hour = tick_time.tm_hour;
  // deal with the 12-hour clock
  if (!clock_is_24h_style()) {
    if (hour > 12 ) {
      hour = hour - 12;
    }
    if (hour == 0) {
      hour = 12;
    }
  }
  #if !HOUR_ZERO
    if (hour >= 10) {
  #endif
    graphics_draw_bitmap_in_rect(ctx, &time_digits[hour / 10].bmp, GRect(4, 0, 60, 82));
  #if !HOUR_ZERO
    }
  #endif
  graphics_draw_bitmap_in_rect(ctx, &time_digits[hour % 10].bmp, GRect(60, 0, 60, 82));
}


// draw the minutes
void paint_minutes(Layer *layer, GContext *ctx) {
  graphics_draw_bitmap_in_rect(ctx, &time_digits[tick_time.tm_min / 10].bmp, GRect(4, 0, 60, 82));
  graphics_draw_bitmap_in_rect(ctx, &time_digits[tick_time.tm_min % 10].bmp, GRect(60, 0, 60, 82));
}


// draw the AM/PM indicator
void paint_ampm(Layer *layer, GContext *ctx) {
  int step = tick_time.tm_hour < 12 ? -1 : 1;
  int base_y = (tick_time.tm_hour < 12) ? 3 : 8;
  for (int i = 0; i < 4; i++) {
    graphics_draw_line(ctx, GPoint(0 + i, base_y + (i * step)), GPoint(6 - i, base_y + (i * step)));
  }
}


// draw the static background of the second indicator
void paint_seconds_background(Layer *layer, GContext *ctx) {
  #if DISPLAY_SECONDS
    // horizontal container lines
    graphics_draw_line(ctx, GPoint(0, 3), GPoint(117, 3));
    graphics_draw_line(ctx, GPoint(0, 6), GPoint(117, 6));

    // halfway indicator (30s)
    graphics_draw_line(ctx, GPoint(59, 0), GPoint(59, 1));
    graphics_draw_line(ctx, GPoint(59, 8), GPoint(59, 9));

    // quarter indicator (15s)
    graphics_draw_line(ctx, GPoint(29, 1), GPoint(29, 1));
    graphics_draw_line(ctx, GPoint(29, 8), GPoint(29, 8));

    // three-quarter indicator (45s)
    graphics_draw_line(ctx, GPoint(89, 1), GPoint(89, 1));
    graphics_draw_line(ctx, GPoint(89, 8), GPoint(89, 8));
  #else
    // horizontal lines
    graphics_draw_line(ctx, GPoint(0, 4), GPoint(117, 4));
    graphics_draw_line(ctx, GPoint(0, 5), GPoint(117, 5));

  #endif
}


// draw the second indicator
void paint_seconds_indicator(Layer *layer, GContext *ctx) {
  #if DISPLAY_SECONDS
    // draw nothing if we're at zero
    if (tick_time.tm_sec > 0) {
      // our indicator is just 2 lines to minimize battery impact while remaining visible
      graphics_draw_line(ctx, GPoint(0, 0), GPoint((tick_time.tm_sec * 2) - 1, 0));
      graphics_draw_line(ctx, GPoint(0, 1), GPoint((tick_time.tm_sec * 2) - 1, 1));
    }
  #endif
}


// initial setup for a layer
void setup_layer(Layer* me, void* painter, Layer* parent, int x, int y, int width, int height) {
  layer_init(me, GRect(x, y, width, height));
  me->update_proc = painter;
  layer_add_child(parent, me);
  layer_mark_dirty(me);
}


// the tick handler
void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  // populate the current time in our time variable
  tick_time = *t->tick_time;
  #if DISPLAY_SECONDS
    layer_mark_dirty(&seconds_indicator_layer);
    if (tick_time.tm_sec == 0) {
  #endif
      layer_mark_dirty(&minutes_layer);
      if (tick_time.tm_min == 0) {
        if (VIBRATE_ON_HOUR) {
          // vibrate on the hour if that option is activated
          vibes_short_pulse();
        }
        layer_mark_dirty(&hours_layer);
        if (!clock_is_24h_style() && (tick_time.tm_hour % 12 == 0)) {
          layer_mark_dirty(&ampm_layer);
        }
        if (tick_time.tm_hour == 0) {
          layer_mark_dirty(&weekday_text_layer);
          layer_mark_dirty(&date_text_layer);
        }
      }
  #if DISPLAY_SECONDS
    }
  #endif
}


// initialize, initialize, INITIALIZE!
void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Big H");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  // load all our resource images (contain a single digit in Roboto font)
  resource_init_current_app(&BIG_H_RESOURCES);
  bmp_init_container(RESOURCE_ID_DATE_0, &date_digits[0]);
  bmp_init_container(RESOURCE_ID_TIME_0, &time_digits[0]);
  bmp_init_container(RESOURCE_ID_DATE_1, &date_digits[1]);
  bmp_init_container(RESOURCE_ID_TIME_1, &time_digits[1]);
  bmp_init_container(RESOURCE_ID_DATE_2, &date_digits[2]);
  bmp_init_container(RESOURCE_ID_TIME_2, &time_digits[2]);
  bmp_init_container(RESOURCE_ID_DATE_3, &date_digits[3]);
  bmp_init_container(RESOURCE_ID_TIME_3, &time_digits[3]);
  bmp_init_container(RESOURCE_ID_DATE_4, &date_digits[4]);
  bmp_init_container(RESOURCE_ID_TIME_4, &time_digits[4]);
  bmp_init_container(RESOURCE_ID_DATE_5, &date_digits[5]);
  bmp_init_container(RESOURCE_ID_TIME_5, &time_digits[5]);
  bmp_init_container(RESOURCE_ID_DATE_6, &date_digits[6]);
  bmp_init_container(RESOURCE_ID_TIME_6, &time_digits[6]);
  bmp_init_container(RESOURCE_ID_DATE_7, &date_digits[7]);
  bmp_init_container(RESOURCE_ID_TIME_7, &time_digits[7]);
  bmp_init_container(RESOURCE_ID_DATE_8, &date_digits[8]);
  bmp_init_container(RESOURCE_ID_TIME_8, &time_digits[8]);
  bmp_init_container(RESOURCE_ID_DATE_9, &date_digits[9]);
  bmp_init_container(RESOURCE_ID_TIME_9, &time_digits[9]);
  #if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_0, &weekday_digits[0][0]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_1, &weekday_digits[0][1]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_2, &weekday_digits[0][2]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_3, &weekday_digits[0][3]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_4, &weekday_digits[0][4]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_5, &weekday_digits[0][5]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_6, &weekday_digits[0][6]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_7, &weekday_digits[0][7]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_8, &weekday_digits[0][8]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_0_9, &weekday_digits[0][9]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_0, &weekday_digits[1][0]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_1, &weekday_digits[1][1]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_2, &weekday_digits[1][2]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_3, &weekday_digits[1][3]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_4, &weekday_digits[1][4]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_5, &weekday_digits[1][5]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_6, &weekday_digits[1][6]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_7, &weekday_digits[1][7]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_8, &weekday_digits[1][8]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_1_9, &weekday_digits[1][9]);
  #elif WEEKDAY_FORMAT == WEEKDAY_FORMAT_EN
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_0, &weekday_name[0]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_1, &weekday_name[1]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_2, &weekday_name[2]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_3, &weekday_name[3]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_4, &weekday_name[4]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_5, &weekday_name[5]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_EN_6, &weekday_name[6]);
  #elif WEEKDAY_FORMAT == WEEKDAY_FORMAT_FR
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_0, &weekday_name[0]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_1, &weekday_name[1]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_2, &weekday_name[2]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_3, &weekday_name[3]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_4, &weekday_name[4]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_5, &weekday_name[5]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_FR_6, &weekday_name[6]);
  #elif WEEKDAY_FORMAT == WEEKDAY_FORMAT_ES
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_0, &weekday_name[0]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_1, &weekday_name[1]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_2, &weekday_name[2]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_3, &weekday_name[3]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_4, &weekday_name[4]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_5, &weekday_name[5]);
    bmp_init_container(RESOURCE_ID_WEEKDAY_ES_6, &weekday_name[6]);
  #endif

  // populate our time variable for our initial status
  get_time(&tick_time);

  // setup the layers
  setup_layer(&hours_layer, paint_hours, &window.layer, 15, 0, 118, 80);
  setup_layer(&minutes_layer, paint_minutes, &window.layer, 15, 88, 118, 80);

  setup_layer(&weekday_background_layer, paint_weekday_background, &window.layer, 0, 0, 15, 168);
  setup_layer(&weekday_text_layer, paint_weekday_text, &weekday_background_layer, 0, 0, 14, 168);

  setup_layer(&date_background_layer, paint_date_background, &window.layer, 133, 0, 11, 168);
  setup_layer(&date_text_layer, paint_date_text, &date_background_layer, 2, 0, 9, 168);

  if (!clock_is_24h_style()) {
    // if we're in 12-hour style, setup the AM/PM layer too
    setup_layer(&ampm_layer, paint_ampm, &window.layer, 71, 80, 7, 12);
  }

  setup_layer(&seconds_background_layer, paint_seconds_background, &window.layer, 15, 81, 118, 10);
  setup_layer(&seconds_indicator_layer, paint_seconds_indicator, &seconds_background_layer, 0, 4, 118, 2);
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  // unload all our images
  for (int i = 0; i < 10; i++) {
    bmp_deinit_container(&date_digits[i]);
    bmp_deinit_container(&time_digits[i]);
    #if WEEKDAY_FORMAT == WEEKDAY_FORMAT_INTL
      for (int j = 0; j < 2; j++) {
        bmp_deinit_container(&weekday_digits[j][i]);
      }
    #endif
  }
  #if WEEKDAY_FORMAT == WEEKDAY_FORMAT_EN || WEEKDAY_FORMAT == WEEKDAY_FORMAT_FR || WEEKDAY_FORMAT == WEEKDAY_FORMAT_ES
    for (int i = 0; i < 7; i++) {
      bmp_deinit_container(&weekday_name[i]);
    }
  #endif
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      #if DISPLAY_SECONDS
        .tick_units = SECOND_UNIT
      #else
        .tick_units = MINUTE_UNIT
      #endif
    }
  };
  app_event_loop(params, &handlers);
}
