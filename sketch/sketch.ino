#include "wifi_credentials.h"
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define NUM_LEDS 54
#define DATA_PIN 13 // LED data pin

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

const char *serverAddress = "94.63.14.247"; //192.168.1.122 LAN, 10.72.73.124 IADE, 94.63.14.247 WLAN
const int serverPort = 5000;  //5000 or 8000

String currentPlayer; 
bool gameStarted = false;

WiFiClient client;

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  initializeBoard();
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

  

  if (!gameStarted) {
    displayMessage("TIC TAC TOE", doc);
  }

  checkWinner();

  
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password); //WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting to connect to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void initializeBoard() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT_PULLUP);
    }
  }
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

String getCurrentPlayer() {
  if (client.connect(serverAddress, serverPort)) {
    client.print("GET /player/current HTTP/1.1\r\n");
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

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    currentPlayer = doc["currentPlayer"].as<String>();  

    Serial.print("O jogador atual é: ");
    Serial.println(currentPlayer);

    return currentPlayer; // Retorna o jogador atual como uma String
  } else {
    Serial.println("Failed to connect to server");
    return "";
  }
}

bool checkAndMarkCells(JsonDocument &doc) {
  JsonArray grid = doc["grid"].as<JsonArray>();

  for (JsonObject cell : grid) {
    const char* marker = cell["marker"];
    if (strcmp(marker, " ") != 0) {
      gameStarted = true;
      break;
    } else {
        gameStarted = false;
    }

  }

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      if (digitalRead(cellPins[x][y]) == LOW) {
        currentPlayer = getCurrentPlayer();
        markCell(x, y, currentPlayer);
        gameStarted = true;
        delay(500); 
      }
    }
  }
  return gameStarted;
}


void displayMessage(String message, JsonDocument &doc) {
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
    if (checkAndMarkCells(doc)) {
      return;
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

void markCell(int x, int y, String marker) {
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

void checkWinner() {
  if (client.connect(serverAddress, serverPort)) {
    client.println("GET /check_winner HTTP/1.1");
    client.print("Host: ");
    client.println(serverAddress);
    client.println("Connection: close");
    client.println();

    
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
    
    // Parse response to check for winner
    if (response.length() > 0) {
      // Parse JSON response
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        JsonObject winnerData = doc["win_cell"];
        if (!winnerData.isNull()) {
          // Extract winner's cells coordinates
          int x1 = winnerData["x"];
          int y1 = winnerData["y"];
          JsonObject winnerData2 = doc["win_cell2"];
          int x2 = winnerData2["x"];
          int y2 = winnerData2["y"];
          JsonObject winnerData3 = doc["win_cell3"];
          int x3 = winnerData3["x"];
          int y3 = winnerData3["y"];
          
          // Make winning cells blink
          blinkWinningCells(x1, y1, x2, y2, x3, y3);
          Serial.println(response);
        }
      }
    }
  }
}
void blinkWinningCells(int x1, int y1, int x2, int y2, int x3, int y3) {
  for (int i = 0; i < 6; i++) {
    int ledIndex1 = ledMappings[x1][y1][i];
    int ledIndex2 = ledMappings[x2][y2][i];
    int ledIndex3 = ledMappings[x3][y3][i];
    
    leds[ledIndex1] = CRGB::Black; // Turn on LED 1
    leds[ledIndex2] = CRGB::Black; // Turn on LED 2
    leds[ledIndex3] = CRGB::Black; // Turn on LED 3
  }
  FastLED.show();
  delay(300); // Blink duration
}