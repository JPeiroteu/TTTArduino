#include <FastLED.h>

#define NUM_LEDS 54
#define DATA_PIN 13

CRGB leds[NUM_LEDS];

const int ledMappings[3][3][6] = {
  {{6, 7, 8, 9, 10, 11}, {24, 25, 26, 27, 28, 29}, {42, 43, 44, 45, 46, 47}},
  {{3, 4, 5, 12, 13, 14}, {21, 22, 23, 30, 31, 32}, {39, 40, 41, 48, 49, 50}},
  {{0, 1, 2, 15, 16, 17}, {18, 19, 20, 33, 34, 35}, {36, 37, 38, 51, 52, 53}}
};

const int cellPins[3][3] = {
  {23, 19, 4},
  {22, 18, 2},
  {21, 5, 15}
};

CRGB currentColor = CRGB::Blue;

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
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      if (digitalRead(cellPins[x][y]) == LOW) {
        Serial.print("Pressure Pad Activited - ");
        Serial.print("X: ");
        Serial.print(x);
        Serial.print(", Y: ");
        Serial.println(y);

        for (int i = 0; i < 6; i++) {
          int ledIndex = ledMappings[x][y][i];
          leds[ledIndex] = currentColor; 
        }
        FastLED.show();

        if (currentColor == CRGB::Blue) {
          currentColor = CRGB::Yellow; 
        } else {
          currentColor = CRGB::Blue; 
        }

        delay(1000);
      }
    }
  }
}
