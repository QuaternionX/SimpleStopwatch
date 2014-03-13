/******************************************************************************
* SimpleStopwatch / Stopwatch watchapp developed by BrandonSoft
* http://brandonsoft.com
* First complete code: January 29, 2014
* *Please don't mind the mess - first pebble app
*
* The Stopwatch allows the user to keep track of time (up to seconds) by using
* the provided large and user-friendly interface. This app also provides 
* an actionbar so the user can pause, reset, and lap the stopwatch. The lap
* button can alternatively be held to view all saved laps.
*
* Enjoy!
*******************************************************************************/
#include <pebble.h>

#define PAUSED_KEY 10
#define TIME_KEY 11

/* variable declarations */
static Window *window;
static Window *lap_window;
static TextLayer *text_layer; /*displays time*/
static ActionBarLayer *action_bar; /*displays buttons*/
static SimpleMenuLayer *lap_layer;
static GBitmap *my_icon_pause, *my_icon_reset, *my_icon_lap, *my_icon_start;
static int seconds = 0;
static int minutes = 0;
static int hours = 0;
/*lap variables*/
static int currentLapIndex = 0;
static bool paused = true;
static char lapTimes[100][12];
static SimpleMenuSection menu_sections[1];
static SimpleMenuItem menu_items[100];

/*Function:   string_to_time(char*)
* Purpose:    Converts a string into the variables that hold time information
*             (useful for loading times from storage/lap list)
* Parameters: timeString - The string to load the times from
* Return:     None
*/
static void string_to_time(char* timeString)
{
  int index = 0;
  char read = timeString[index];
  while (read != '\0')
  {
    if (read == 'h')
      hours = (10*(timeString[index-2] - '0') + (timeString[index-1] - '0'));
    if (read == 'm')
      minutes = (10*(timeString[index-2] - '0') + (timeString[index-1] - '0'));
    if (read == 's')
      seconds = (10*(timeString[index-2] - '0') + (timeString[index-1] - '0'));

    ++index;
    read = timeString[index];
  }
}
/*Function:   vtom(int,int,int)
* Purpose:    To convert individual hours, minutes, and seconds values 
*             into a readable String for the user to see. This is done
*             through a String that simply appends the digits of the numbers
*             in their correct locations.
* Parameters: sec  - the seconds to put into string
*             min  - the minutes to put into string
*             hour - the hours to put into string
* Return:     The String containing the readable time.
*/
static char * vtom(int sec, int min, int hour)
{
  /* create appended time number and empty String template */
  int time = (hour * 10000) + (min * 100) + sec;
  static char arr[] = "00h\n00m\n00s";
  int i = 9; /* used to scroll through time template */

  while (time > 0)
  {
    arr[i] = (time % 10) + '0'; /* add time digits (conv to char) */
    time /= 10; /* remove inserted digit from number */

    /*logic to skip a spot (if it is \n or letter spot)*/
    if (i % 4 == 1)
      i--;
    else if (i % 4 == 0)
      i-=3;
  }
  /* return the formatted string */
  return arr;
}


/*Function:   vitom(char*,int)
* Purpose:    To convert the parameter array into an applicable time
*             String by altering its individual elements (just as vtom
*             does, but without the creation of the array).
* Parameters: arr  - The array to alter
*             time - The time to put into the array
* Return:     Nothing
*/
void vitom(char* arr, int time)
{
  int i = 9; /* to scroll through array */

  while (time > 0)
  {
    arr[i] = (time % 10) + '0'; /* insert char format of last digit */
    time /= 10; /* trim time */

    /*logic to skip a spot (if it is \n or letter spot)*/
    if (i % 4 == 1)
      i--;
    else if (i % 4 == 0)
      i-=3;
  }
}


/*Function:   switch_resume_icon()
* Purpose:    To switch the resume icon from a play icon to a pause 
*             icon and vice/versa.
* Parameters: None
* Return:     None
*/
static void switch_resume_icon()
{
  if (paused)
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_start);
  else
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_pause);
}


/*Function:   handle_second_tick(struct tm*,TimeUnits)
* Purpose:    To handle the meat of the timer and actually increment
*             all elements of the stored time. Increments seconds, minutes,
*             and hours. Also handles the updating of the time text.
* Parameters: tick_time     - The current time of the watch
*             units_changed - The units of tick of this function
* Return:     Nothing.
*/
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed)
{
  if (paused) /*don't update time if paused is pressed */
    return;

  seconds++;

  /* now we need to handle minutes and hours */
  if (seconds >= 60)
  {
    minutes++;
    seconds = 0;
  }
  if (minutes >= 60)
  {
    hours++;
    minutes = 0;
  }

  /* update text layer */
  text_layer_set_text(text_layer, vtom(seconds,minutes,hours));
}

/*Function:   lap_handle(ClickRecognizerRef, void*)
* Purpose:    To handle when the lap (bottom) button is pressed by adding
*             the current time to the lap array. This is done through
*             usage of the vitom method.
* Parameters: recognizer - The button that is pressed
*             context    - Not really sure? But required.
* Return:     Nothing.
*/
void lap_handle(ClickRecognizerRef recognizer, void *context)
{
  int i = currentLapIndex;  /*store start point for loop */
  int time = (hours*10000) + (minutes*100) + seconds; /*conv time to 1 int*/

  /* reset the array pos */
  lapTimes[i][0] = '0';
  lapTimes[i][1] = '0';
  lapTimes[i][2] = 'h';
  lapTimes[i][3] = ':';
  lapTimes[i][4] = '0';
  lapTimes[i][5] = '0';
  lapTimes[i][6] = 'm';
  lapTimes[i][7] = ':';
  lapTimes[i][8] = '0';
  lapTimes[i][9] = '0';
  lapTimes[i][10] = 's';
  lapTimes[i][11] = '\0';
  
  /* populate array pos accurately using vitom*/
  vitom(lapTimes[i], time);

  /*open spot for next lap */
  currentLapIndex++;
}

/*Function:   reset_laps()
* Purpose:    To reset all of the laps stored in the laps array by 
*             set the hour indicator to a faulty value, this clearing
*             it from the displaying on the lap menu list thing! Clever!
* Parameters: none
* Return:     none
*/
void reset_laps()
{
  int i = 0; /* To loop through all existing laps*/

  for (i = 0; i < currentLapIndex; i++)
  {
    lapTimes[i][0] = '0';
    lapTimes[i][1] = '0';
    lapTimes[i][2] = 'n'; /* alter 'h' to 'n' to make it unrecognized */
    lapTimes[i][3] = ':';
    lapTimes[i][4] = '0';
    lapTimes[i][5] = '0';
    lapTimes[i][6] = 'm';
    lapTimes[i][7] = ':';
    lapTimes[i][8] = '0';
    lapTimes[i][9] = '0';
    lapTimes[i][10] = 's';
    lapTimes[i][11] = '\0';
  }
}
    

/*Function:   pause_handle(ClickRecognizerRef,void*)
* Purpose:    To handle the pressing of the pause (top) button by
*             stopping the timer and switching the icon.
* Parameters: recognizer - The button that was pressed
*             context    - No idea? It is required.
* Return:     none.
*/
void pause_handle(ClickRecognizerRef recognizer, void *context)
{
  paused = !paused; /* switch paused state */
  switch_resume_icon(); /* switch resume icon */
}

/*Function:   reset_handle(ClickRecognizerRef,void*)
* Purpose:    To handle the pressing of the reset (middle) button
*             by resetting the secs, minutes, and hours variables,
*             clearing the lap array, and updating the text.
* Parameters: recognizer - The button that was pressed.
*             context    - No idea? It is required.
* Return:     None.
*/
void reset_handle(ClickRecognizerRef recognizer, void *context)
{
  /* reset time */
  seconds = 0;
  minutes = 0;
  hours = 0;

  reset_laps(); /*reset laps */

  /*pause the stopwatch*/
  paused = true;
  switch_resume_icon(); /* switch resume icon */

  /*update text*/
  text_layer_set_text(text_layer, "00h\n00m\n00s");
}

/*Function:   lap_window_load(Window*)
* Purpose:    To load the lap window when the lap button is held 
*             pressed by creating the menu, adding it, and displaying
*             the window.
* Parameters: window - the lap window being loaded
* Return:     none.
*/
static void lap_window_load(Window *window)
{
  /* get reference of root layer */
  Layer *window_layer = window_get_root_layer(window);

  /* get frame, but menu into it */
  GRect bounds = layer_get_frame(window_layer);
  lap_layer = simple_menu_layer_create(bounds, window, menu_sections, 1, NULL);

  /* add menu to window */
  layer_add_child(window_layer, simple_menu_layer_get_layer(lap_layer));
}

/*Function:   lap_window_unload(Window*)
* Purpose:    To unload the lap window when the back button is pressed
*             by destroying the menu and the window.
* Parameters: window - the lap window to unload
* Return:     none.
*/
static void lap_window_unload(Window *window)
{
  /* destroy all the things. */
  simple_menu_layer_destroy(lap_layer);
  window_destroy(window);
}

/*Function:   lap_menu_handle(ClickRecognizerRef,void*)
* Purpose:    To handle when the lap menu is created (because the lap button
*             is held down). Therefore, this function tallies all stored
*             lap times, inserts them into item/category arrays, and then
*             displays them in a simple menu window that is pushed onto
*             the window stack.
* Parameters: recognizer - the data regarding the button pressed
*             context    - I don't know? It is required.
*/
void lap_menu_handle(ClickRecognizerRef recognizer, void *context)
{
    /* used to count the number of laps and scroll through them */
    int numLaps = 0;
    int i = 0;

    /*count number of recognizable laps*/
    while (lapTimes[numLaps][2] == 'h')
      numLaps++;

    /*put all laps into menuitem objects*/
    for (i = 0; i < numLaps; i++)
    {
      menu_items[i] = (SimpleMenuItem){
        .title = &lapTimes[i][0], /*objects have time as title*/
      };
    }

    /* put all laps into a unified section*/
    menu_sections[0] = (SimpleMenuSection){
      .num_items = numLaps,
      .items = menu_items,
    };

    /*create the lap window*/
    lap_window = window_create();
    
    /* set proper load/unload methods for the lap window */
    window_set_window_handlers(lap_window, (WindowHandlers) {
        .load = lap_window_load,
        .unload = lap_window_unload,
        });

    /* push window to window stack! */
    window_stack_push(lap_window, true);
}

/*Function:   click_config_provider(void*)
* Purpose:    To assign all the buttons to their handler functions so 
*             all of the stopwatch works in a preferable manner and all 
*             functionality is seamless.
* Parameters: context - No idea? It is required.
* Return:     None.
*/
void click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) lap_handle);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) pause_handle);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) reset_handle);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, 
                                                 (ClickHandler) lap_menu_handle, 
                                                 NULL); 
}

/*Function:   window_load(Window*)
* Purpose:    To load the main window onto the screen by creating all elements
*             of the window. This means the text layer, the icons, the
*             actionbar, etc. Then, the window is pushed onto the window stack.
* Parameters: window - The window to load
* Return:     None.
*/
static void window_load(Window *window) {
  /* create reference of root window */
  Layer *window_layer = window_get_root_layer(window);
  /* initialize all timer vars */
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

  /* set default text layer */
  text_layer = text_layer_create(GRect(5, 0, 100, 160));
  text_layer_set_text(text_layer, "00h\n00m\n00s");
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_layer)); /*add*/

  /*initialize images*/
  my_icon_start = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESUME);
  my_icon_reset = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESET);
  my_icon_lap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LAP);
  my_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);

  /*initialize side action bar*/
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, my_icon_start);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, my_icon_reset);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, my_icon_lap);

  /* initialize the timer (handlers and tickers) */
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}

/*Function:   window_unload(Window*)
* Purpose:    To unload the main window by destroying all of its
*             resources. This includes the text layer and action bar
* Parameters: window - the window to unload
* Return:     none.
*/
static void window_unload(Window *window)
{
  text_layer_destroy(text_layer);
  action_bar_layer_destroy(action_bar);
}

/*Function:   save_data(void)
* Purpose:    This method allows for laps and current time/pause state to be
*             saved within the watch so they are resumed when the app is
*             reopned.
* Parameters: None
* Return:     None
*/
static void save_data(void)
{
  persist_write_string(TIME_KEY, vtom(seconds,minutes,hours));
  persist_write_bool(PAUSED_KEY, paused);
}

/*Function:   load_data(void)
* Purpose:    This method allows for laps/current time/pause state to be resumed
*             from when the user last quit the app
* Parameters: None
* Return:     None
*/
static void load_data(void)
{
  /*default values*/
  char* time = "00h:00m:00s";
  bool lpaused = true;

  /*load values if exist*/
  if (persist_exists(TIME_KEY))
    persist_read_string(TIME_KEY, time, PERSIST_STRING_MAX_LENGTH);
  if (persist_exists(PAUSED_KEY))
    lpaused = persist_read_bool(PAUSED_KEY);

  paused = lpaused;
  string_to_time(time);


  switch_resume_icon();
}

/*Function:   init(void)
* Purpose:    initialize the main window by creating it, adding the handlers
*             to all of the proper functions, and pushing it to the window stack
* Parameters: void (req.)
* Return:     None.
*/
static void init(void) {
  /*create the window */
  window = window_create();

  /*assign the handlers*/
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });

  /* load data */
  load_data();

  /*push the window onto the stack*/
  window_stack_push(window, true);
}

/*Function:   deinit(void)
* Purpose:    To deinitialize the main window by destroying the window
*             and all elements of said window. This means all icons and the
*             window itself
* Parameters: void (req.)
* Return:     none.
*/
static void deinit(void) {
  save_data(); /*save all the data before-hand*/

  /*deinitialize all images*/
  gbitmap_destroy(my_icon_start);
  gbitmap_destroy(my_icon_reset);
  gbitmap_destroy(my_icon_lap);
  gbitmap_destroy(my_icon_pause);

  /*deinitialize the main window*/
  window_destroy(window);
}

/*Function:   main(void)
* Purpose:    To start the application by running all initializers, making
*             a quick write to the log, and starting the loop. Once the 
*             app loop is complete (or an exit occurs), the program is deinited
* Parameters: void (req.)
* Return:     none
*/
int main(void) {
  /*init everything*/
  init();
  /*write to log*/
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  /*start main loop*/
  app_event_loop();

  /*deinit after everything*/
  deinit();
}

