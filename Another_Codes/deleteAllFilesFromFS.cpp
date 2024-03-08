#include <Arduino.h>

void printMemoryInfo() {
  int freeMemory = ESP.getFreeHeap();
  int minFreeMemory = ESP.getMinFreeHeap();
  int maxAllocatableMemory = ESP.getMaxAllocHeap();
  uint32_t flashSize = ESP.getFlashChipSize();

  Serial.println("===== Informace o paměti =====");
  Serial.println("Volná paměť: " + String(freeMemory) + " bytes");
  Serial.println("Minimálně volná paměť: " + String(minFreeMemory) + " bytes");
  Serial.println("Maximálně alokovatelná paměť: " + String(maxAllocatableMemory) + " bytes");
  Serial.println("Velikost flash paměti: " + String(flashSize) + " bytes");
  Serial.println("==============================");
}

void setup() {
  Serial.begin(9600);
  printMemoryInfo();
}

void loop() {
  // Hlavní smyčka loop() je prázdná
}
