/*
cellPins[0][0] 10 | cellPins[0][1] 7 | cellPins[0][2] 4
-----------------------------------------------
cellPins[1][0] 9 | cellPins[1][1] 6 | cellPins[1][2] 3
-----------------------------------------------
cellPins[2][0] 8 | cellPins[2][1] 5 | cellPins[2][2] 2
*/
const int cellPins[3][3] = {
  {2, 3, 4},
  {5, 6, 7},
  {8, 9, 10}
};

void setup() {
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(cellPins[x][y], INPUT);
    }
  }
  
  Serial.begin(9600);
}

void loop() {
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