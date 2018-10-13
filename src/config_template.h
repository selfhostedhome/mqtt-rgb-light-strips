#define SSID                          "<SSID>"
#define PASSWORD                      "<PASSWORD>"

// MQTT Settings
#define HOSTNAME                      "esp8266-light-strip"
#define MQTT_SERVER                   "192.168.1.9"

#define AVAILABILITY_TOPIC            "office/rgb1/light"
#define STATE_TOPIC                   "office/rgb1/light/status"
#define COMMAND_TOPIC                 "office/rgb1/light/switch"

#define BRIGHTNESS_STATE_TOPIC        "office/rgb1/brightness/status"
#define BRIGHTNESS_COMMAND_TOPIC      "office/rgb1/brightness/set"

#define RGB_STATE_TOPIC               "office/rgb1/rgb/status"
#define RGB_COMMAND_TOPIC             "office/rgb1/rgb/set"

#define EFFECT_STATE_TOPIC            "office/rgb1/effect/status"
#define EFFECT_COMMAND_TOPIC          "office/rgb1/effect/set"

// OTA Settings
#define OTA_PORT             8266

// LED Config
#include <FastLED.h>

#define DATA_PIN    5
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG
#define NUM_LEDS    300

#define BRIGHTNESS          90
#define FRAMES_PER_SECOND   300
