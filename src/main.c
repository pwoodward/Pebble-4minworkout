#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static AppTimer *timer;

// Icons
GBitmap *play_bitmap, *stop_bitmap;

// Timer defaults
static const int EXERCISETIME = 20000;
static const int RESTTIME = 10000;

// Static strings
static const char *EXERCISE[] = {"Running in Place", 
                                 "Jumping Jacks", 
                                 "Jumping Squats",
                                 "Push Ups",
                                 "Bicycle Crunch",
                                 "Burpees",
                                 "Mason Twist",
                                 "Mountain Climbers"};

// Rep counter
static int rep = -1;
// Timer type. 0 rest, 1 exercise, 2 end
static int type = 0;

// Reset the app state
void reset_state(void)
{
  rep = -1;
  type = 0;
  if(timer != NULL)
  {
    app_timer_cancel(timer);
  }
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, play_bitmap);
  text_layer_set_text(text_layer, "4 Minute Workout");
}

/*
 * Timer callbacks
 */
static void timer_callback(void *data) 
{
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer happened %d for timer %d", rep, type);
  // check rep is set correctly
  if(rep < 0)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid rep setting found %d", rep);
    return;
  }
  
  if(type == 0)
  {
    // Rest timer finished
    type = 1;
    vibes_long_pulse();
  
    text_layer_set_text(text_layer, "Go");
    
    // Start exercise timer
    timer = app_timer_register(EXERCISETIME, timer_callback, NULL);
  }
  else if (type == 1)
  {
    // Exercise timer finished
    vibes_long_pulse();
    type = 0;
    rep++;
    
    // start rest timer if needed
    if(rep >= 8)
    {
      text_layer_set_text(text_layer, "Routine finished. Well done!");
      // Wait 5 seconds and reset status back to default
      type = 2;
      timer = app_timer_register(RESTTIME/2, timer_callback, NULL);
    }
    else
    {
      text_layer_set_text(text_layer, EXERCISE[rep]);
      timer = app_timer_register(RESTTIME, timer_callback, NULL);
    }
  }
  else if(type == 2)
  {
    reset_state();
  }
  else
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid type setting: %d", type);
  }
}

/*
 * Button handlers
 */
void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
}
 
void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
}
 
void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  // Check to make sure we are running
  if(rep == -1)
  {
    // Reset the counters
    rep = 0;
    type = 0;
    
    text_layer_set_text(text_layer, EXERCISE[rep]);
    
    // Start the timer reps. 5 sec rest to start.
    timer = app_timer_register(RESTTIME/2, timer_callback, NULL);
    
    // Set the icon to stop
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, stop_bitmap);
  }
  else
  {
    reset_state();
  }
}

void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/*
 * Window Load and Unload
 */
void window_load()
{
  // Create action bar icons
  play_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
  stop_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STOP);
  
  // Create the Action Bar
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  
  // Create the Text layer
  text_layer = text_layer_create(GRect(5,38,144 - ACTION_BAR_WIDTH - 5,168 - 38));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorBlack);
  
  // Add the text layer to the window
  layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
  
  // Reset the status to default
  reset_state();
}

void window_unload()
{
  // Clean Up the window load
  text_layer_destroy(text_layer);
  action_bar_layer_destroy(action_bar);
  //Destroy GBitmaps
  gbitmap_destroy(play_bitmap);
  gbitmap_destroy(stop_bitmap);
}

/*
 * Init and setup
 */

void init()
{
  // Init stuff here
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  //window_set_click_config_provider(window, click_config_provider);
  window_stack_push(window, true);
}

void deinit()
{
  // Cleanup
  window_destroy(window);
}


int main(void)
{
  init();
  app_event_loop();
  deinit();
}