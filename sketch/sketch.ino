#include "wifi_credentials.h"
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>

#define NUM_LEDS 54
#define DATA_PIN 13 // LED data pin
#define RESET_PIN 12 // Button pin for reset

CRGB leds[NUM_LEDS];

// LED mappings for each cell
const int ledMappings[3][3][6] = {
  {{6, 7, 8, 9, 10, 11}, {24, 25, 26, 27, 28, 29}, {42, 43, 44, 45, 46, 47}},
  {{3, 4, 5, 12, 13, 14}, {21, 22, 23, 30, 31, 32}, {39, 40, 41, 48, 49, 50}},
  {{0, 1, 2, 15, 16, 17}, {18, 19, 20, 33, 34, 35}, {36, 37, 38, 51, 52, 53}}
};

// Cells pins mapping
const int cellPins[3][3] = {
  {23, 19, 4},
  {22, 18, 2},
  {21, 5, 15}
};

const char *serverAddress = "94.63.14.247"; //192.168.1.122 LAN, 10.72.73.124 IADE, 94.63.14.247 WLAN
const int serverPort = 5000;  //5000 or 8000

String currentPlayer; 
bool boardState = false;
bool playerState = false;

DynamicJsonDocument doc(1024); 
WiFiClient client;

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  initializeBoard();
  pinMode(RESET_PIN, INPUT_PULLUP);
}

void loop() {
  getBoardState();
  updateLeds(doc);
  checkAndMarkCells(doc);
  checkWinner();

  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("Reset button pressed. Resetting game...");
    resetGame();
    delay(1000);
  }
}

// WiFi connection function
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting to connect to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

// Initialize the LED and board setup
void initializeBoard() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT_PULLUP);
    }
  }
}

// Fetch the current board state from the server
void getBoardState() {
  if (!boardState) {
    boardState = true; 
    getCurrentPlayer();
    AsyncClient *asyncClient = new AsyncClient();

    asyncClient->onConnect([](void* arg, AsyncClient* c) {
      Serial.println("Connected, getting board ...");
      c->write(("GET game/0/board HTTP/1.1\r\nHost: " + String(serverAddress) + "\r\nConnection: close\r\n\r\n").c_str());
    }, nullptr);

    asyncClient->onData([](void* arg, AsyncClient* c, void* data, size_t len) {
      String boardState = String((char*)data).substring(0, len);
      Serial.print("Data: ");
      Serial.println(boardState);

      // Deserialize the board state into a temporary JSON document
      DynamicJsonDocument tempDoc(1024);
      deserializeJson(tempDoc, boardState);
      doc.clear();
      doc = tempDoc;
    }, nullptr);

    asyncClient->onDisconnect([](void* arg, AsyncClient* c) {
      Serial.println("Disconnected");
      c->close();
      delete c; // Free memory after disconnection
      boardState = false; // Reset flag to allow future updates
    }, nullptr);

    if (!asyncClient->connect(serverAddress, serverPort)) {
      Serial.println("Connection failed");
      delete asyncClient;
      boardState = false;
    } else {
      Serial.println("Board state update in progress...");
    }
  }
}

// Fetch the current players turn
void getCurrentPlayer() {
  if (!playerState) {
    playerState = true;

    AsyncClient* asyncClient = new AsyncClient();

    asyncClient->onConnect([](void* arg, AsyncClient* c) {
      Serial.println("Connected, getting current player ...");
      c->write(("GET game/0/player/current HTTP/1.1\r\nHost: " + String(serverAddress) + "\r\nConnection: close\r\n\r\n").c_str());
    }, nullptr);

    asyncClient->onData([](void* arg, AsyncClient* c, void* data, size_t len) {
      String response = String((char*)data).substring(0, len);
      Serial.print("Data: ");
      Serial.println(response);

      // Deserialize the current player response
      DynamicJsonDocument tempDoc(1024);
      deserializeJson(tempDoc, response);
      currentPlayer = tempDoc["currentPlayer"].as<String>();
    }, nullptr);

    asyncClient->onDisconnect([](void* arg, AsyncClient* c) {
      Serial.println("Disconnected");
      c->close();
      delete c; // Free memory after disconnection
      playerState = false;
    }, nullptr);

    if (!asyncClient->connect(serverAddress, serverPort)) {
      Serial.println("Connection failed");
      delete asyncClient;
      playerState = false;
    } else {
      Serial.println("Current player update in progress...");
    }
  }
}

// Update the LEDs based on the current board state
void updateLeds(JsonDocument &doc) {
  JsonArray grid = doc["grid"].as<JsonArray>();

  for (JsonObject cell : grid) {
    int x = cell["x"];
    int y = cell["y"];
    const char* marker = cell["marker"];

    updateCellLeds(x, y, marker);
  }
  FastLED.show();
}

// Update LEDs for a specific cell based on the marker
void updateCellLeds(int x, int y, const char* marker) {
  CRGB color;

  if (strcmp(marker, "X") == 0) {
    color = CRGB::Blue;
  } else if (strcmp(marker, "O") == 0) {
    color = CRGB::Green; 
  }

  for (int i = 0; i < 6; i++) {
    int ledIndex = ledMappings[x][y][i];
    leds[ledIndex] = color;
  }
}

// Check and mark cells when players interact with the pressure pads
void checkAndMarkCells(JsonDocument &doc) {
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      if (digitalRead(cellPins[x][y]) == LOW) {
        const char* marker = currentPlayer.c_str(); 
        updateCellLeds(x, y, marker);
        FastLED.show(); 
        markCell(x, y, currentPlayer);
      }
    }
  }
}

// Send a request to the server to mark a cell with the current players marker
void markCell(int x, int y, String marker) {
  String data = "x=" + String(x) + "&y=" + String(y) + "&mark=" + String(marker);

  if (client.connect(serverAddress, serverPort)) {
    client.printf("POST game/0/cell/mark HTTP/1.1\r\nHost: %s\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s",
      serverAddress, data.length(), data.c_str());

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

// Check if there is a winner
void checkWinner() {
  if (client.connect(serverAddress, serverPort)) {
    client.printf("GET game/0/check_winner HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", serverAddress);

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
    
    if (response.length() > 0) {
      DynamicJsonDocument tempDoc(1024); // Create a temporary JSON document
      DeserializationError error = deserializeJson(tempDoc, response);
      
      if (!error) {
        JsonObject winnerData = tempDoc["win_cell"];
        if (!winnerData.isNull()) {
          int x1 = winnerData["x"];
          int y1 = winnerData["y"];
          JsonObject winnerData2 = tempDoc["win_cell2"];
          int x2 = winnerData2["x"];
          int y2 = winnerData2["y"];
          JsonObject winnerData3 = tempDoc["win_cell3"];
          int x3 = winnerData3["x"];
          int y3 = winnerData3["y"];
          
          blinkWinningCells(x1, y1, x2, y2, x3, y3);
          Serial.println(response);
        }
      } else {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
    }
  }
}

// Blink the LEDs of the winning cells
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
  delay(1500); // Blink duration
}

// Reset the game by sending a reset request to the server
void resetGame() {
  Serial.println("Resetting game...");

  if (client.connect(serverAddress, serverPort)) {
    client.printf("GET game/0/reset_board HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", serverAddress);

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }

    client.stop();
    Serial.println("Game reset successfully.");
  } else {
    Serial.println("Failed to connect to server.");
  }
}