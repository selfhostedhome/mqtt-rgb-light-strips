#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <MSGEQ7.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);
CMSGEQ7<MSGEQ7_SMOOTH, MSGEQ7_PIN_RESET, MSGEQ7_PIN_STROBE, MSGEQ7_PIN_ANALOG>
    MSGEQ7;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(HOSTNAME)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish(AVAILABILITY_TOPIC, "online");

            client.subscribe(COMMAND_TOPIC);
            client.subscribe(BRIGHTNESS_COMMAND_TOPIC);
            client.subscribe(RGB_COMMAND_TOPIC);
            client.subscribe(EFFECT_COMMAND_TOPIC);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setupOTA() {
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(HOSTNAME);

    ArduinoOTA.onStart([]() { Serial.println("Starting"); });
    ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}

CRGB leds[NUM_LEDS];

int Brightness;
uint8_t Hue = 0; // rotating "base color" used by many of the patterns

int Red;
int Green;
int Blue;

bool TurnOn = false;
bool TurnOff = false;
bool UpdateBrightness = false;
bool UpdateColorFill = false;
bool UpdateEffect = false;

int MusicEffectCycle = 0;

typedef enum Effects {
    RAINBOW,
    RAINBOW_WITH_GLITTER,
    CONFETTI,
    SINELON,
    BPM,
    JUGGLE,
    CANDYCANE,
    PARTY_COLORS,
    ROTATE_PARTY_COLORS,
    MUSIC_RAINBOW,
    MUSIC_RGB,
    MUSIC_ROTATE_PARTY_COLORS,
    MUSIC_CYCLE,
} Effect;

// Globals for LED State Control
Effect CurrentEffect;

const char *EffectStrings[] = {"rainbow",
                               "rainbow_with_glitter",
                               "confetti",
                               "sinelon",
                               "bpm",
                               "juggle",
                               "candycane",
                               "party_colors",
                               "rotate_party_colors",
                               "music_rainbow",
                               "music_rgb",
                               "music_rotate_party_colors",
                               "music_cycle"};

void callback(char *topic, byte *payload, unsigned int length) {

    char payloadString[20];

    char colorStrings[3][4];

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    memcpy(payloadString, payload, length);
    payloadString[length] = '\0';
    Serial.println(payloadString);

    // Process the command that was received
    if (strcmp(topic, COMMAND_TOPIC) == 0) {
        Serial.println("Processing the command...");
        if (strncmp(payloadString, "on", length) == 0) {
            TurnOn = true;
        } else {
            TurnOff = true;
        }
    } else if (strcmp(topic, BRIGHTNESS_COMMAND_TOPIC) == 0) {
        Serial.println("Processing the brightness...");
        UpdateBrightness = true;
        Brightness = atoi(payloadString);
    } else if (strcmp(topic, RGB_COMMAND_TOPIC) == 0) {
        Serial.println("Processing the RGB...");
        UpdateColorFill = true;
        unsigned int component = 0;
        unsigned int j = 0;
        for (unsigned int i = 0; i < length; i++) {
            if (payload[i] == ',') {
                colorStrings[component][j] = '\0';
                component++;
                j = 0;
            } else {
                colorStrings[component][j] = payload[i];
                j++;
            }
        }
        colorStrings[component][j] = '\0';

        Red = atoi(colorStrings[0]);
        Green = atoi(colorStrings[1]);
        Blue = atoi(colorStrings[2]);

    } else if (strcmp(topic, EFFECT_COMMAND_TOPIC) == 0) {
        Serial.println("Processing the Effect...");
        UpdateEffect = true;

        if (strcmp(payloadString, "rainbow") == 0) {
            CurrentEffect = RAINBOW;
        } else if (strcmp(payloadString, "rainbow_with_glitter") == 0) {
            CurrentEffect = RAINBOW_WITH_GLITTER;
        } else if (strcmp(payloadString, "confetti") == 0) {
            CurrentEffect = CONFETTI;
        } else if (strcmp(payloadString, "sinelon") == 0) {
            CurrentEffect = SINELON;
        } else if (strcmp(payloadString, "bpm") == 0) {
            CurrentEffect = BPM;
        } else if (strcmp(payloadString, "juggle") == 0) {
            CurrentEffect = JUGGLE;
        } else if (strcmp(payloadString, "candycane") == 0) {
            CurrentEffect = CANDYCANE;
        } else if (strcmp(payloadString, "party_colors") == 0) {
            CurrentEffect = PARTY_COLORS;
        } else if (strcmp(payloadString, "rotate_party_colors") == 0) {
            CurrentEffect = ROTATE_PARTY_COLORS;
        } else if (strcmp(payloadString, "music_rainbow") == 0) {
            CurrentEffect = MUSIC_RAINBOW;
        } else if (strcmp(payloadString, "music_rgb") == 0) {
            CurrentEffect = MUSIC_RGB;
        } else if (strcmp(payloadString, "music_rotate_party_colors") == 0) {
            CurrentEffect = MUSIC_ROTATE_PARTY_COLORS;
        } else if (strcmp(payloadString, "music_cycle") == 0) {
            CurrentEffect = MUSIC_CYCLE;
        }
    }
}

void setup() {
    // This will set the Audio IC ready for reading
    MSGEQ7.begin();

    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);

    Serial.print("Connecting...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(MQTT_SERVER, 1883);
    client.setCallback(callback);

    setupOTA();

    // Setting up heartbeat
    pinMode(LED_BUILTIN, OUTPUT);

    Brightness = BRIGHTNESS;
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show(); // needed to reset leds to zero
}

void candyCane() {
    static uint8_t startIndex = 0;
    const CRGBPalette16 currentPalettestriped = CRGBPalette16(
        CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
        CRGB::Red, CRGB::Red, CRGB::White, CRGB::White, CRGB::White,
        CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White);

    startIndex = startIndex + 1;
    fill_palette(leds, NUM_LEDS, startIndex, 16, currentPalettestriped, 255,
                 LINEARBLEND);
}

void partyColors() {
    static uint8_t startIndex = 0;

    startIndex = startIndex + 1;
    fill_palette(leds, NUM_LEDS, startIndex, 16, PartyColors_p, 255,
                 LINEARBLEND);
}

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, Hue, 7);
}

uint8_t getBassReading() {
    // Get values for lowest frequencies
    uint8_t freq0 = MSGEQ7.get(MSGEQ7_0, 0);
    uint8_t freq1 = MSGEQ7.get(MSGEQ7_1, 0);
    freq0 = mapNoise(freq0);
    freq1 = mapNoise(freq0);

    // Use whichever frequency is being used more
    freq0 = max(freq0, freq1);

    // Map the values so there is always at least 20 brightness
    freq0 = map(freq0, 0, 255, 20, 255);

    return freq0;
}

void musicRainbow() {
    bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

    if (newReading) {
        uint8_t freq = getBassReading();
        FastLED.setBrightness(freq);
        rainbow();
    }
}

void musicPartyColor() {
    bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

    if (newReading) {
        uint8_t freq = getBassReading();
        FastLED.setBrightness(freq);
        partyColors();
    }
}

void musicRGB() {
    bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

    if (newReading) {
        uint8_t freq1 = MSGEQ7.get(MSGEQ7_1, 0);
        uint8_t freq3 = MSGEQ7.get(MSGEQ7_3, 0);
        uint8_t freq5 = MSGEQ7.get(MSGEQ7_5, 0);

        freq1 = mapNoise(freq1);
        freq3 = mapNoise(freq3);
        freq5 = mapNoise(freq5);

        fill_solid(leds, NUM_LEDS, CRGB(freq1, freq3, freq5));
        fadeToBlackBy(leds, NUM_LEDS, 10);
    }
}

void addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void confetti() {
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(Hue + random8(64), 200, 255);
}

void sinelon() {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    leds[pos] += CHSV(Hue, 255, 192);
}

void bpm() {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) { // 9948
        leds[i] =
            ColorFromPalette(palette, Hue + (i * 2), beat - Hue + (i * 10));
    }
}

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void rotatePartyColors() {
    static uint8_t colorIndex = 0;
    static uint8_t delay = 0;

    fill_solid(&(leds[0]), NUM_LEDS,
               ColorFromPalette(PartyColors_p, colorIndex, 200));
    if (delay == 8) {
        colorIndex++;
        delay = 0;
    } else {
        delay++;
    }
}

void musicRotatePartyColors() {

    bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

    if (newReading) {
        uint8_t freq = getBassReading();
        rotatePartyColors();
        FastLED.setBrightness(freq);
    }
}

typedef void (*SimpleEffectList[])();

// List of music effects. The 'musicCycle' effect cycles through these
SimpleEffectList MusicEffectFxns = {
    musicRainbow,
    musicRGB,
    musicRotatePartyColors,
};

void musicCycle() { MusicEffectFxns[MusicEffectCycle](); }

// List of effects. Each is defined as a separate function
SimpleEffectList EffectFxns = {rainbow,
                               rainbowWithGlitter,
                               confetti,
                               sinelon,
                               bpm,
                               juggle,
                               candyCane,
                               partyColors,
                               rotatePartyColors,
                               musicRainbow,
                               musicRGB,
                               musicRotatePartyColors,
                               musicCycle};

String rgbString() {
    String rgb;
    rgb = String(Red) + ',' + String(Green) + ',' + String(Blue);
    return rgb;
}

void loop() {

    static bool showLeds = true;
    static bool runEffect = false;

    if (!client.connected()) {
        reconnect();
    }

    // Process MQTT tasks
    client.loop();

    if (TurnOn) {
        TurnOn = false;
        showLeds = true;
        FastLED.setBrightness(Brightness);
        client.publish(STATE_TOPIC, "on");
        client.publish(BRIGHTNESS_STATE_TOPIC, String(Brightness).c_str());
    }
    if (TurnOff) {
        TurnOff = false;
        showLeds = false;
        FastLED.setBrightness(0);
        client.publish(STATE_TOPIC, "off");
        client.publish(BRIGHTNESS_STATE_TOPIC, String(0).c_str());
    }
    if (UpdateBrightness) {
        UpdateBrightness = false;
        FastLED.setBrightness(Brightness);
        client.publish(BRIGHTNESS_STATE_TOPIC, String(Brightness).c_str());
    }
    if (UpdateColorFill) {
        UpdateColorFill = false;
        runEffect = false;
        Serial.println(rgbString());
        fill_solid(&(leds[0]), NUM_LEDS, CRGB(Red, Green, Blue));
        client.publish(RGB_STATE_TOPIC, rgbString().c_str());
        client.publish(EFFECT_STATE_TOPIC, String("").c_str());
    }
    if (UpdateEffect) {
        UpdateEffect = false;
        runEffect = true;
        client.publish(EFFECT_STATE_TOPIC,
                       String(EffectStrings[CurrentEffect]).c_str());
        client.publish(RGB_STATE_TOPIC, String("").c_str());
    }

    // Handle OTA
    ArduinoOTA.handle();

    // Fill the LED array with an effect if an effect is active
    if (runEffect) {
        EffectFxns[CurrentEffect]();
    }

    if (showLeds) {
        FastLED.show();
    }

    // Insert a delay to keep the framerate modest
    delay(1000 / FRAMES_PER_SECOND);

    // Slowly cycle the "base color" through the rainbow
    EVERY_N_MILLISECONDS(20) { Hue++; }

    // Every 10 minutes cycle music effects
    EVERY_N_SECONDS(600) {}

    // Heartbeat
    EVERY_N_MILLISECONDS(3000) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    // Every 10 minutes cycle music effects
    EVERY_N_SECONDS(600) {
        MusicEffectCycle++;
        if (MusicEffectCycle == ARRAY_SIZE(MusicEffectFxns)) {
            MusicEffectCycle = 0;
        }
    }
}
