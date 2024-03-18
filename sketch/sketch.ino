#include <FastLED.h>

#define NUM_LEDS 54 
#define DATA_PIN 6  

CRGB leds[NUM_LEDS]; 

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  
}

void loop() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);

  FastLED.show();

  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  FastLED.show();

  delay(1000);
}
