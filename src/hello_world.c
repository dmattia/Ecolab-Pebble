#include <pebble.h>
#include "status_bar.h"
	
#define BAR_HEIGHT 25
#define ICON_WIDTH_HEIGHT 25
	
enum {
  KEY_TEMPERATURE = 0,
	KEY_PRICE = 2,
	KEY_CHANGE = 3,
	KEY_NEGATIVE = 4
};
	
Window *window;
static TextLayer* s_time_layer;
static CustomStatusBarLayer *custom_status_bar;
static TextLayer* s_stock_price_layer;
static BitmapLayer * s_background_layer;
static GBitmap * s_background_bitmap;
static int goingDown; // 1 if stock change is negative, 0 otherwise

char * const_to_mutable(const char* original) {
	unsigned int i = 0, length = strlen(original);
	char* mutableVersion = malloc(sizeof(char) * length);
	for(i = 0; i < length; ++i) {
		mutableVersion[i] = original[i];
	}
	return mutableVersion;
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char time_buffer[] = "00:00";
	static char date_buffer[] = "01/01";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style()) {
    // Use 24 hour format
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
	
	strftime(date_buffer, sizeof(date_buffer), "%m/%d", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, time_buffer);
	
	custom_status_bar_layer_set_text(custom_status_bar, CSB_TEXT_RIGHT, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
	//Ask to update stock and weather info every 5 minutes
	if(tick_time->tm_min % 5 == 0) {
 	  // Begin dictionary
 	  DictionaryIterator *iter;
 	  app_message_outbox_begin(&iter);
	
	  // Add a key-value pair
	  dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char weather_layer_buffer[8];
	static char stock_price_buffer[8];
	static char stock_change_buffer[8];
	static char stock_layer_buffer[16];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%dF", (int)t->value->int32);
      break;
		case KEY_PRICE:
			snprintf(stock_price_buffer, sizeof(stock_price_buffer), "%s", t->value->cstring);
			break;
		case KEY_CHANGE:
			snprintf(stock_change_buffer, sizeof(stock_change_buffer), "%s", t->value->cstring);
			break;
		case KEY_NEGATIVE:
			goingDown = (int)t->value->int32;
			#ifdef PBL_COLOR
				APP_LOG(APP_LOG_LEVEL_DEBUG,"goingDown in preprocessor: %d",goingDown);
				if(goingDown) {
					text_layer_set_text_color(s_stock_price_layer, GColorRed);
				} else {
					text_layer_set_text_color(s_stock_price_layer, GColorJaegerGreen);
		    }
			#endif
			break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }  
	
  // Assemble full string and display
	#ifndef PBL_COLOR
	if(goingDown) {
		snprintf(stock_layer_buffer, sizeof(stock_layer_buffer), "%s -%s", stock_price_buffer, stock_change_buffer);
	} else {
		snprintf(stock_layer_buffer, sizeof(stock_layer_buffer), "%s +%s", stock_price_buffer, stock_change_buffer);
	}
	#else
		snprintf(stock_layer_buffer, sizeof(stock_layer_buffer), "%s %s", stock_price_buffer, stock_change_buffer);
	#endif
	text_layer_set_text(s_stock_price_layer, stock_layer_buffer);
	custom_status_bar_layer_set_text(custom_status_bar, CSB_TEXT_LEFT, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void handle_init(void) {
	//Create bitmap
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ECOLAB);
	s_background_layer = bitmap_layer_create(GRect(0, 6 + BAR_HEIGHT, 144, 162 - BAR_HEIGHT));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	 // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	// Create a window and other layers
	window = window_create();
	s_time_layer = text_layer_create(GRect(0,110 + BAR_HEIGHT,144,29));
	s_stock_price_layer = text_layer_create(GRect(0, 80 + BAR_HEIGHT, 144, 29));
	#ifdef PBL_COLOR
		custom_status_bar = custom_status_bar_layer_create(BAR_HEIGHT, GColorVividCerulean, GColorWhite, ICON_WIDTH_HEIGHT);
	#else
		custom_status_bar = custom_status_bar_layer_create(BAR_HEIGHT, GColorBlack, GColorWhite, ICON_WIDTH_HEIGHT);
	#endif
	
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Setup for stock layer
	text_layer_set_text(s_stock_price_layer, "...");
	text_layer_set_font(s_stock_price_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_stock_price_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_stock_price_layer, GColorClear);
	
	// Setup for time layer
	text_layer_set_text(s_time_layer, "00:00");
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	#ifdef PBL_COLOR
		text_layer_set_text_color(s_time_layer, GColorPictonBlue);	
	#endif
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_time_layer, GColorClear);

	// Setup Title Layer
	custom_status_bar_layer_set_text(custom_status_bar, CSB_TEXT_CENTER, "Ecolab");
	
	// Add layers to window
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_stock_price_layer));
	layer_add_child(window_get_root_layer(window), custom_status_bar);

	// Push the window
	window_stack_push(window, true);
}

void handle_deinit(void) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_stock_price_layer);
	custom_status_bar_layer_destroy(custom_status_bar);
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
