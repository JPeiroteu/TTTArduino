#include <FastLED.h>

#define NUM_LEDS 54
#define DATA_PIN 13

CRGB leds[NUM_LEDS];

const int cellPins[3][3] = {
  {23, 19, 4},
  {22, 18, 2},
  {21, 5, 15}
};

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT_PULLUP);
    }
  }
  Serial.begin(9600);
}

void loop() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  
  FastLED.show();

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      if (digitalRead(cellPins[x][y]) == LOW) {
        Serial.print("Test: ");
        Serial.print(cellPins[x][y]);
        Serial.print("\n");
      }
    }
  }
}
