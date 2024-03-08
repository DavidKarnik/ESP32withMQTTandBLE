#include <Arduino.h>

void setup() {
  Serial.begin(9600);
}

void loop() {

  // Informace o procesoru:
  Serial.println("Informace o procesoru:");
  // Serial.print(xPortGetCoreID());
  Serial.print("Taktovací frekvence procesoru: ");
  Serial.println(ESP.getCpuFreqMHz());

  // Informace o Chipu:
  Serial.println("Informace o Chipu:");
  // Serial.println("Fyzická jádra: " + ESP.getChipCores());
  Serial.print("Model: ");
  Serial.println(ESP.getChipModel());
  // Serial.println(ESP.getChipRevision());

  // Získání informací o velikosti flash paměti
  Serial.print("Velikost flash paměti: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");

  // Získání informací o dostupné flash paměti
  Serial.print("Dostupná flash paměť: ");
  Serial.print(ESP.getFreeSketchSpace() / 1024);
  Serial.println(" KB");

  // Získání informací o volné paměti
  Serial.print("Volná paměť: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");

// // Informace o WiFi:
// Serial.println("Informace o WiFi:");
// Serial.print("Stav WiFi: ");
// Serial.println(WiFi.status());
// Serial.print("Připojen k WiFi: ");
// Serial.println(WiFi.isConnected());
// // Získání MAC adresy zařízení
// Serial.print("MAC adresa: ");
// Serial.println(WiFi.macAddress());

  // Verze Arduino Core pro ESP32:
  Serial.print("Verze SDK: ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("ESP-IDF Version: ");
  Serial.println(ESP_IDF_VERSION);

  // Informace o systémovém čase:
  Serial.print("Aktuální čas od spuštění: ");
  Serial.println(millis());

  // Získání aktuálního času ve formě takty jádra
  Serial.print("Aktuální čas: ");
  Serial.println(xTaskGetTickCount());

  Serial.print("---------------------------------\n");

  delay(5000); // Počkejte 5 sekund a opakujte
}
