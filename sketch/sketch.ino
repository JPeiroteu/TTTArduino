#include "wifi_credentials.h"
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define NUM_LEDS 54
#define DATA_PIN 13 // LEDs data pin

CRGB leds[NUM_LEDS];

// LED mappings for each cell
const int ledMappings[3][3][6] = {
  {{6, 7, 8, 9, 10, 11}, {24, 25, 26, 27, 28, 29}, {42, 43, 44, 45, 46, 47}},
  {{3, 4, 5, 12, 13, 14}, {21, 22, 23, 30, 31, 32}, {39, 40, 41, 48, 49, 50}},
  {{0, 1, 2, 15, 16, 17}, {18, 19, 20, 33, 34, 35}, {36, 37, 38, 51, 52, 53}}
};

// Cells pins
const int cellPins[3][3] = {
  {23, 19, 4},
  {22, 18, 2},
  {21, 5, 15}
};

const char *serverAddress = "192.168.1.122";
const int serverPort = 5000;

char currentMarker = 'X'; // X blue LEDs and O green LEDs

bool gameStarted = false;

WiFiClient client;

void setup() {
  Serial.begin(9600);

  connectToWiFi();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT_PULLUP);
    }
  }
}


void loop() {
  String boardState = getBoardState();
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, boardState);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Update LEDs based on board state
  updateLeds(doc);

  // Check any pressed cell
  checkAndMarkCells(doc);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
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
          response += line; // JSON body
        }
      }
    }
    client.stop();
    Serial.println(response);
    return response;
  } else {
    Serial.println("Failed to connect to server");
    return "";
  }
}

void updateLeds(JsonDocument &doc) {
  JsonArray grid = doc["grid"].as<JsonArray>();
  
  if (!gameStarted) {
    displayMessage("TIC TAC TOE");
  }

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
  delay(100);
}

void checkAndMarkCells(JsonDocument &doc) {
  JsonArray grid = doc["grid"].as<JsonArray>();

  if (!gameStarted) {
    for (JsonObject cell : grid) {
      const char* marker = cell["marker"];
      if (strcmp(marker, " ") != 0) {
        gameStarted = true;
        break;
      }
    }
  }

  if (gameStarted) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        if (digitalRead(cellPins[x][y]) == LOW) {
          markCell(x, y, currentMarker);
          currentMarker = (currentMarker == 'X') ? 'O' : 'X';
          delay(500); 
        }
      }
    }
  }
}

void displayMessage(String message) {
  int letterIndex = 0;
  for (int i = 0; i < message.length(); i++) {
    switch (message[i]) {
      case 'T':
        displayLetterT();
        break;
      case 'I':
        displayLetterI();
        break;
      case 'C':
        displayLetterC();
        break;
      case 'A':
        displayLetterA();
        break;
      case 'O':
        displayLetterO();
        break;
      case 'E':
        displayLetterE();
        break;
      case ' ':
        break;
    }
    delay(1000);
    clearLeds(); 
  }
}

void displayLetterT() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[0][0][i]] = CRGB::Blue;
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[0][2][i]] = CRGB::Blue;
    leds[ledMappings[1][1][i]] = CRGB::Blue;
    leds[ledMappings[2][1][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void displayLetterI() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[1][1][i]] = CRGB::Blue;
    leds[ledMappings[2][1][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void displayLetterC() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[0][2][i]] = CRGB::Blue;
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[0][0][i]] = CRGB::Blue;
    leds[ledMappings[1][0][i]] = CRGB::Blue;
    leds[ledMappings[2][0][i]] = CRGB::Blue;
    leds[ledMappings[2][1][i]] = CRGB::Blue;
    leds[ledMappings[2][2][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void displayLetterA() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[2][0][i]] = CRGB::Blue;
    leds[ledMappings[1][0][i]] = CRGB::Blue;
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[1][2][i]] = CRGB::Blue;
    leds[ledMappings[2][2][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void displayLetterO() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[0][0][i]] = CRGB::Blue;
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[0][2][i]] = CRGB::Blue;
    leds[ledMappings[1][2][i]] = CRGB::Blue;
    leds[ledMappings[2][2][i]] = CRGB::Blue;
    leds[ledMappings[2][1][i]] = CRGB::Blue;
    leds[ledMappings[2][0][i]] = CRGB::Blue;
    leds[ledMappings[1][0][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void displayLetterE() {
  for (int i = 0; i < 6; i++) {
    leds[ledMappings[0][0][i]] = CRGB::Blue;
    leds[ledMappings[0][1][i]] = CRGB::Blue;
    leds[ledMappings[0][2][i]] = CRGB::Blue;
    leds[ledMappings[1][2][i]] = CRGB::Blue;
    leds[ledMappings[2][2][i]] = CRGB::Blue;
    leds[ledMappings[2][1][i]] = CRGB::Blue;
    leds[ledMappings[2][0][i]] = CRGB::Blue;
    leds[ledMappings[1][0][i]] = CRGB::Blue;
    leds[ledMappings[1][1][i]] = CRGB::Blue;
  }
  FastLED.show();
  delay(100); 
}

void clearLeds() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show(); 
}

void markCell(int x, int y, char marker) {
  String postData = "x=" + String(x) + "&y=" + String(y) + "&mark=" + String(marker);

  sendPostRequest("/cell/mark", postData);
}

void sendPostRequest(String path, String data) {
  if (client.connect(serverAddress, serverPort)) {
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);

    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
  } else {
    Serial.println("Failed to connect to server");
  }
}