#include <pebble.h>

Window *window;
static TextLayer *s_text_layer;
static TextLayer* s_time_layer;
static TextLayer* s_weather_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

void handle_init(void) {
	// Create a window and other layers
	window = window_create();
	s_text_layer = text_layer_create(GRect(0, 0, 144, 154));
	s_time_layer = text_layer_create(GRect(0,40,144,114));
	
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Setup for Text Layer
	text_layer_set_text(s_text_layer, "Ecolab");
	text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_text_layer, GColorClear);
	
	// Setup for time layer
	text_layer_set_text(s_time_layer, "00:00");
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_time_layer, GColorClear);
	
	// Add layers to window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

	// Push the window
	window_stack_push(window, true);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_text_layer);
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
