#include <pebble.h>
/*
* User settings
* Show Clock
* Show Weather
* Autohide
*/

// Config
#define QTP_K_SHOW_TIME 1
#define QTP_K_AUTOHIDE 4
#define QTP_K_DEGREES_F 8
#define QTP_K_INVERT 16
#define QTP_K_SUBSCRIBE 32
#define QTP_K_VIBRATE 64

#define QTP_SCREEN_WIDTH        144
#define QTP_SCREEN_HEIGHT       168

#define QTP_PADDING_Y 18
#define QTP_PADDING_X 11
#define QTP_BAT_ICON_SIZE 122
#define QTP_BATTERY_BASE_Y 132



// Items
static int QTP_WINDOW_TIMEOUT = 2000;
static Window *qtp_window;
static bool qtp_is_showing;
static TextLayer *qtp_battery_text_layer;
static TextLayer *qtp_bluetooth_text_layer;
static AppTimer *qtp_hide_timer;
static GBitmap *qtp_bluetooth_image;
static GBitmap *qtp_battery_images[3];
static GBitmap *qtp_battery_image;
static BitmapLayer *qtp_battery_image_layer;
static BitmapLayer *qtp_bluetooth_image_layer;
static int qtp_conf;
static bool qtp_bluetooth_status;



static const int QTP_BATTERY_ICONS[] = {
	RESOURCE_ID_IMAGE_0,
	RESOURCE_ID_IMAGE_1,
	RESOURCE_ID_IMAGE_2
};


// Methods
void qtp_setup();
void qtp_app_deinit();

void qtp_show();
void qtp_hide();
void qtp_timeout();

void qtp_tap_handler(AccelAxisType axis, int32_t direction);
void qtp_click_config_provider(Window *window);
void qtp_back_click_responder(ClickRecognizerRef recognizer, void *context);

void qtp_bluetooth_callback(bool connected);

void qtp_update_battery_status(bool mark_dirty);
void qtp_update_bluetooth_status(bool mark_dirty);

void qtp_init();
void qtp_deinit();

// Helpers
bool qtp_is_show_time();
bool qtp_is_autohide();
bool qtp_is_degrees_f();
bool qtp_is_invert();
bool qtp_should_vibrate();

void qtp_set_config(int config);
void qtp_set_timeout(int timeout);
void qtp_init_bluetooth_status(bool status);

int qtp_battery_y();
int qtp_bluetooth_y();
int qtp_weather_y();
