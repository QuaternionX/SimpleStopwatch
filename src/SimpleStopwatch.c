#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static int counter = 0;
static bool paused = true;

static char *itoa(int num) {
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  if(num > 0) {
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    if (length == 2)
      length = 1;
    buff[0] = '0'; //ensures extra 0 in front!
    buff[1] = '0';
    for(i = length; i >= 0; i--) {
      buff[i] = '0' + (num % 10);
      num /= 10;
    }
    buff[length+1] = '\0'; // can't forget the null byte to properly end our string
  }
  else
    return "00";
  return string;
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed)
{
  if (paused)
    return;

  counter++;
  static char catStr[9] = {};
  catStr[0] = '\0';
  strcat(catStr, itoa(counter/360));
  strcat(catStr, "h\n"); //don't know how else to do it
  strcat(catStr, itoa(counter/60) - ((counter/360)*60));
  strcat(catStr, "m\n");
  strcat(catStr, itoa(counter - ((counter/60)*60) - ((counter/360)*60)));
  strcat(catStr, "s");

  text_layer_set_text(text_layer, catStr);
}

void lap_handle(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(text_layer, "LAP!");
}

void pause_handle(ClickRecognizerRef recognizer, void *context)
{
  paused = !paused;
}

void reset_handle(ClickRecognizerRef recognizer, void *context)
{
  counter = 0;
  text_layer_set_text(text_layer, "00h\n00m\n00s");
}

void click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) lap_handle);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) pause_handle);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) reset_handle);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer = text_layer_create(GRect(5, 0, 100, 160));
  text_layer_set_text(text_layer, "00h\n00m\n00s");
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  //initialize side action bar
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}

