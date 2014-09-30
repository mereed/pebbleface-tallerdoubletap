#include "QTPlus.h"

GColor qtp_background_color = GColorBlack;

static AppTimer *display_timer;
static AppTimer *tap_timer;
int cur_day = -1;

static TextLayer *qtp_time_layer;
static TextLayer *qtp_date_layer;
static TextLayer *qtp_day_layer;

#define TAP_TIME 4000
static bool is_tapped_waiting;

/* Initialize listeners to show and hide Quick Tap Plus as well as update data */
void qtp_setup() {
    qtp_is_showing = false;
	accel_tap_service_subscribe(&qtp_tap_handler);
	
	qtp_battery_images[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_0);
	qtp_battery_images[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_1);
	qtp_battery_images[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_2);
	
	qtp_battery_image = qtp_battery_images[0];
	
	qtp_bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_QTP_IMG_BT);
	
	qtp_bluetooth_status = bluetooth_connection_service_peek();
	bluetooth_connection_service_subscribe( qtp_bluetooth_callback );
	
}

static void timer_callback() {
  is_tapped_waiting = false;
 
  // DEBUG ONLY - VIBE TRIGGERS ANOTHER TAP
  // vibes_short_pulse();
}

void double_tap() {
	
  // ACTION TO BE PERFORMED AFTER DOUBLE TAP
	if (qtp_is_showing) {
		qtp_hide();
	} else {
		qtp_show();
	}
	qtp_is_showing = !qtp_is_showing;	
}



/* Handle taps from the hardware */
void qtp_tap_handler(AccelAxisType axis, int32_t direction) {

	//if (axis == ACCEL_AXIS_Z) {
    if (!is_tapped_waiting) {
      is_tapped_waiting = true;
      tap_timer = app_timer_register(TAP_TIME, timer_callback, NULL);
    }
    
    else {
      double_tap();
 
      app_timer_cancel(tap_timer);
      is_tapped_waiting = false;
    //}
  }
}  
	



/* Subscribe to taps and pass them to the handler */
void qtp_click_config_provider(Window *window) {
	window_single_click_subscribe(BUTTON_ID_BACK, qtp_back_click_responder);
}

/* Unusued. Subscribe to back button to exit */
void qtp_back_click_responder(ClickRecognizerRef recognizer, void *context) {
	qtp_hide();
}

/* Update the text layer for the battery status */
void qtp_update_battery_status(bool mark_dirty) {
	BatteryChargeState charge_state = battery_state_service_peek();
	static char battery_text[] = "100%";
	snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);

	if (charge_state.charge_percent >= 70) {
		qtp_battery_image = qtp_battery_images[2];
	} else if (charge_state.charge_percent <= 30) {
		qtp_battery_image = qtp_battery_images[0];
	} else {
		qtp_battery_image = qtp_battery_images[1];
	}
	bitmap_layer_set_bitmap(qtp_battery_image_layer, qtp_battery_image);

	text_layer_set_text(qtp_battery_text_layer, battery_text);
	if (mark_dirty) {
		layer_mark_dirty(text_layer_get_layer(qtp_battery_text_layer));
		layer_mark_dirty(bitmap_layer_get_layer(qtp_battery_image_layer));
	}
}

/* Update the text layer for the bluetooth status */
void qtp_update_bluetooth_status(bool mark_dirty) {
	static char bluetooth_text[] = "Not Paired";

	if (qtp_bluetooth_status) {
		snprintf(bluetooth_text, sizeof(bluetooth_text), "Paired");
	} else {
		snprintf(bluetooth_text, sizeof(bluetooth_text), "Un-Paired");
	}

	text_layer_set_text(qtp_bluetooth_text_layer, bluetooth_text);
	if (mark_dirty) {
		layer_mark_dirty(text_layer_get_layer(qtp_bluetooth_text_layer));
	}
}

void qtp_update_date(struct tm *tick_time) {

	static char date_text[] = "xxxxxxxxx 00";
    static char wday_text[] = "Week 00";
		
	// Only update the date when it's changed.
    int new_cur_day = tick_time->tm_year*1000 + tick_time->tm_yday;
    if (new_cur_day != cur_day) {
        cur_day = new_cur_day;
	}
        strftime(date_text, sizeof(date_text), "%B %e", tick_time);
        text_layer_set_text(qtp_date_layer, date_text);

        strftime(wday_text, sizeof(wday_text), "Week %V", tick_time);
        text_layer_set_text(qtp_day_layer, wday_text);

}

void force_update(void) {
    time_t now = time(NULL);
    qtp_update_date(localtime(&now));
}

void qtp_bluetooth_callback(bool connected) {
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "QTP: bluetooth status callback: %d", connected);
	
		qtp_bluetooth_status = connected;

		if (connected) {
			vibes_short_pulse();
		} else {
			vibes_double_pulse();
		} 

	if (qtp_is_showing) {
		qtp_update_bluetooth_status(false);
	}
}


/* Auto-hide the window after a certain time */
void qtp_timeout() {
	qtp_hide();
	qtp_is_showing = false;
}

/* Create the QTPlus Window and initialize the layres */
void qtp_init() {
	qtp_window = window_create();

	qtp_background_color  = GColorBlack;
    window_set_background_color(qtp_window, qtp_background_color);


	
	GRect date_frame = GRect( 0,135,144,50 );
		qtp_date_layer = text_layer_create(date_frame);
	    text_layer_set_text_color(qtp_date_layer, GColorWhite);
        text_layer_set_background_color(qtp_date_layer, GColorClear);
		text_layer_set_text_alignment(qtp_date_layer, GTextAlignmentCenter);
		text_layer_set_font(qtp_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	    //text_layer_set_font(qtp_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MOVIE_45)));
		layer_add_child(window_get_root_layer(qtp_window), text_layer_get_layer(qtp_date_layer));
	
	GRect day_frame = GRect( 0,2,144,50 );
		qtp_day_layer = text_layer_create(day_frame);
	    text_layer_set_text_color(qtp_day_layer, GColorWhite);
        text_layer_set_background_color(qtp_day_layer, GColorClear);
		text_layer_set_text_alignment(qtp_day_layer, GTextAlignmentCenter);
		text_layer_set_font(qtp_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	    //text_layer_set_font(qtp_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MOVIE_45)));
		layer_add_child(window_get_root_layer(qtp_window), text_layer_get_layer(qtp_day_layer));
	
	
	/* Battery Logo layer */
	GRect battery_logo_frame = GRect( 27, 50, 32, 16 );
	qtp_battery_image_layer = bitmap_layer_create(battery_logo_frame);
	bitmap_layer_set_bitmap(qtp_battery_image_layer, qtp_battery_image);
	bitmap_layer_set_alignment(qtp_battery_image_layer, GAlignCenter);
	layer_add_child(window_get_root_layer(qtp_window), bitmap_layer_get_layer(qtp_battery_image_layer)); 

	/* Battery Status text layer */
	GRect battery_frame = GRect( 70,40,80,30 );
	qtp_battery_text_layer =  text_layer_create(battery_frame);
    text_layer_set_text_color(qtp_battery_text_layer, GColorWhite);
    text_layer_set_background_color(qtp_battery_text_layer, GColorClear);
	text_layer_set_font(qtp_battery_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment(qtp_battery_text_layer, GTextAlignmentLeft);
	
	qtp_update_battery_status(false);
	layer_add_child(window_get_root_layer(qtp_window), text_layer_get_layer(qtp_battery_text_layer));

	
	/* Bluetooth Logo layer */
	GRect bluetooth_logo_frame = GRect(30, 90, 32, 32);
	qtp_bluetooth_image_layer = bitmap_layer_create(bluetooth_logo_frame);
	bitmap_layer_set_bitmap(qtp_bluetooth_image_layer, qtp_bluetooth_image);
	bitmap_layer_set_alignment(qtp_bluetooth_image_layer, GAlignCenter);
	layer_add_child(window_get_root_layer(qtp_window), bitmap_layer_get_layer(qtp_bluetooth_image_layer)); 


	/* Bluetooth Status text layer */

	GRect bluetooth_frame = GRect(70,90,80, 26);
	qtp_bluetooth_text_layer =  text_layer_create(bluetooth_frame);
	text_layer_set_text_color(qtp_bluetooth_text_layer, GColorWhite);
    text_layer_set_background_color(qtp_bluetooth_text_layer, GColorClear);
	text_layer_set_font(qtp_bluetooth_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	qtp_update_bluetooth_status(false);
	text_layer_set_text_alignment(qtp_bluetooth_text_layer, GTextAlignmentLeft);
	layer_add_child(window_get_root_layer(qtp_window), text_layer_get_layer(qtp_bluetooth_text_layer));

	
	force_update();

}


/* Deallocate QTPlus items when window is hidden */
void qtp_deinit() {

	bitmap_layer_destroy(qtp_battery_image_layer);
	window_destroy(qtp_window);
	
	//if (qtp_is_autohide()) {
	//	app_timer_cancel(qtp_hide_timer);
	//}
}

/* Deallocate persistent QTPlus items when watchface exits */
void qtp_app_deinit() {
	

	gbitmap_destroy(qtp_battery_images[0]);
	gbitmap_destroy(qtp_battery_images[1]);
	gbitmap_destroy(qtp_battery_images[2]);

}

/* Create window, layers, text. Display QTPlus */
void qtp_show() {
	qtp_init();
	window_stack_push(qtp_window, true);
	
	app_timer_cancel(display_timer);
	display_timer = app_timer_register(4000, qtp_timeout, NULL);
	
	//if (qtp_is_autohide()) {
	//qtp_hide_timer = app_timer_register(QTP_WINDOW_TIMEOUT, qtp_timeout, NULL);
	//}
}

/* Hide QTPlus. Free memory */
void qtp_hide() {
	window_stack_pop(true);
	qtp_deinit();
}


bool qtp_is_show_time() {
	return (qtp_conf & QTP_K_SHOW_TIME) == QTP_K_SHOW_TIME;
}

bool qtp_is_autohide() {
	return (qtp_conf & QTP_K_AUTOHIDE) == QTP_K_AUTOHIDE;
}

bool qtp_is_invert() {
	return (qtp_conf & QTP_K_INVERT) == QTP_K_INVERT;
}

bool qtp_should_vibrate() {
	return (qtp_conf & QTP_K_VIBRATE) == QTP_K_VIBRATE;
}


void qtp_set_config(int config) {
	qtp_conf = config;
}

void qtp_set_timeout(int timeout) {
	QTP_WINDOW_TIMEOUT = timeout;	
}

void qtp_init_bluetooth_status(bool status) {
	qtp_bluetooth_status = status;
}