#include <FastLED.h>
#include <Arduino.h>
#include <projectDefs.h>

const int NUM_LEDS = 2;


CRGB leds[NUM_LEDS];

void led_init() {FastLED.addLeds<WS2812, leds_pin, RGB>(leds, 2);}

void led1_rgb(uint8_t red_col, uint8_t green_col, uint8_t blue_col){
    leds[0] = CRGB(red_col, green_col, blue_col);
}

void led2_rgb(uint8_t red_col, uint8_t green_col, uint8_t blue_col){
    leds[1] = CRGB(red_col, green_col, blue_col);
}

void led_update(){
    FastLED.show();
}