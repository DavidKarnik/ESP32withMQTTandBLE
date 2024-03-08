#include <Arduino.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Získání informací o paměti
  int freeMemory = ESP.getFreeHeap();
  int minFreeMemory = ESP.getMinFreeHeap();
  int maxAllocatableMemory = ESP.getMaxAllocHeap();
  uint32_t flashSize = ESP.getFlashChipSize();

  // Získání informací o CPU
  uint32_t cpuFreq = ESP.getCpuFreqMHz();
  uint32_t cycleCount = ESP.getCycleCount();
  uint32_t elapsedTime = millis();

  // Výpis informací na sériový monitor
  Serial.println("===== Informace o ESP32 =====");
  Serial.println("Volná paměť: " + String(freeMemory) + " bytes");
  Serial.println("Minimálně volná paměť: " + String(minFreeMemory) + " bytes");
  Serial.println("Maximálně alokovatelná paměť: " + String(maxAllocatableMemory) + " bytes");
  Serial.println("Frekvence CPU: " + String(cpuFreq) + " MHz");
  Serial.println("Velikost flash paměti: " + String(flashSize) + " bytes");
  Serial.println("Četnost cyklů: " + String(cycleCount));
  Serial.println("Uplynulý čas: " + String(elapsedTime) + " ms");
  Serial.println("==============================");

  delay(5000); // Pauza 5 sekund
}

// ===== Informace o ESP32 =====
// Volná paměť: 336768 bytes
// Minimálně volná paměť: 331432 bytes
// Maximálně alokovatelná paměť: 112628 bytes
// Frekvence CPU: 240 MHz
// Velikost flash paměti: 4194304 bytes
// Četnost cyklů: 971040016
// Uplynulý čas: 57737 ms
// ==============================


// RAM:   [          ]   4.2% (used 22168 bytes from 532480 bytes)
// Flash: [==        ]  20.2% (used 264221 bytes from 1310720 bytes)


// data poslat přes..
// rs485 <- ... shield
// bluetooth - data in/out
// kdo je master / slave ... kolik esp možno prřipojit
// virtuální sériová kominukace

// různé servery posílat

// jedno jádro nonstop modbus
