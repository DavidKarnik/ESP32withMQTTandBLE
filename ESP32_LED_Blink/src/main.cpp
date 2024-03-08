#include <Arduino.h>

void setup() {
  pinMode(13, OUTPUT); // Nastavení pinu 13 jako výstup
}

void loop() {
  digitalWrite(13, HIGH); // Zapnutí LED
  delay(2000); // Pauza 1 sekunda
  digitalWrite(13, LOW); // Vypnutí LED
  delay(2000); // Pauza 1 sekunda
}
