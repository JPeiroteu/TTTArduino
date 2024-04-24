# TicTacToe IoT Controller

The Arduino code serves as the brain behind the physical game board, enabling seamless interaction between players and the web-based TicTacToe game. Controlled by an ESP32 microcontroller, the physical board features LED strips and handmade pressure pads, allowing physical players to visualize and execute moves during gameplay.

## Overview

The ESP32 controller facilitates real-time communication and control of the LED strip integrated into the physical game board. This code complements the main [TicTacToe project](https://github.com/JPeiroteu/tic-tac-toe/), synchronizing player moves between physical and digital gameplay plays.

## Getting Started

### Required Components
- [1x ESP32 Microcontroller](https://www.amazon.de/-/en/ESP-32S-Development-Bluetooth-Microcontroller-ESP-WROOM-32/dp/B07XH45MWW/ref=sr_1_19?crid=G8IZGQUA1J2V&keywords=esp32&qid=1651326820&sprefix=esp32%2Caps%2C95&sr=8-19)
- [1x Breadboard](https://www.amazon.de/-/en/AZDelivery-Breadboard-Kit-Compatible-Book/dp/B078JGQKWP/ref=sr_1_6?crid=26659W6VDD48R&keywords=breadboards&qid=1651318781&sprefix=breadboards%2Caps%2C107&sr=8-6)
- Jumper wires
- Aluminum Foil (for pressure pads)
- Aluminum Plate (for grounding)
- Pieces of wood
- 9x 10K resistors (pull-up)
- [1x 1m LED RGB WS2812B](https://amzn.eu/d/iV5lQFY) 

### Installation Steps
1. **Install the Arduino IDE.**
2. **Install ESP32 Add-on in Arduino IDE:**
   - Open Arduino IDE and navigate to `File > Preferences`.
   - Add the following URLs to the "Additional Board Manager URLs" field:
     - `https://dl.espressif.com/dl/package_esp32_index.json`
     - `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Click OK to save the changes.
   - Go to `Tools > Board > Boards Manager…` and search for "ESP32". Install the "ESP32 by Espressif Systems" package.

3. **Install Required Libraries:**
   - WiFi
   - FastLED
   - AsyncTCP
   - ArduinoJson
   - ArduinoUnit

Install these libraries using the Arduino Library Manager (`Sketch > Include Library > Manage Libraries`).

## Circuit Design
![Coming Soon]()

## Usage

1. **Download the Project:**
   - Clone or download the [TTTArduino](https://github.com/JPeiroteu/TTTArduino) repository.
2. **Setup ESP32 Board:**
   - Connect the ESP32 board to your computer via USB.
   - Open the downloaded project in Arduino IDE.
   - Select the appropriate board (DOIT ESP32 DEVKIT V1) from `Tools > Board`.
   - Choose the correct COM port from `Tools > Port`.
3. **Upload the Code:**
   - Press the Upload button in Arduino IDE to compile and upload the code to your ESP32 board.
4. **Wait for Upload Completion:**
   - Once the upload is complete, you will see a "Done uploading" message.
5. **Start Interacting:**
   - With the ESP32 connected to the physical board and with the Flask server working, players can now interact with the pressure pads to make moves in the TicTacToe game. The LED strips provide visual feedback on player moves and game state changes.

## Contribution

Contributions to the TicTacToe IoT Controller project are welcome! Whether it's bug fixes, feature enhancements, or new ideas, feel free to open an issue or submit a pull request to contribute to the project's development.
