#include "wifi_credentials.h"
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoJson.h>

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

const char* serverAddress = "192.168.1.122";
const int serverPort = 5000;

WiFiClient client;

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT_PULLUP);
    }
  }

  Serial.begin(9600);

  connectToWiFi();
}

void loop() {
  String boardState = getBoardState();
  DynamicJsonDocument doc(1024);

  JsonArray grid = doc["grid"].as<JsonArray>();
  for (JsonObject cell : grid) {
    int x = cell["x"];
    int y = cell["y"];
    const char* marker = cell["marker"];

    for (int i = 0; i < 6; i++) {
      int ledIndex = ledMappings[x][y][i];
      leds[ledIndex] = (strcmp(marker, "X") == 0) ? CRGB::Blue : (strcmp(marker, "O") == 0) ? CRGB::Green : CRGB::Black;
    }
  }
  
  FastLED.show();
  delay(1000); 
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting to connect to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

String getBoardState() {
  if (client.connect(serverAddress, serverPort)) {
    client.print("GET /board HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(serverAddress);
    client.println("\r\nConnection: close\r\n");
    
    String response = "";
    bool isBody = false;
    
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          isBody = true;
        } else if (isBody) {
          response += line;
        }
      }
    }
    
    client.stop();
    return response;
  } else {
    return "";
  }
}