#include <Arduino.h>

String sharedString;  // Společná proměnná pro obě jádra
SemaphoreHandle_t mutex;  // Mutex pro synchronizaci přístupu k proměnné

void task1(void *pvParameters) {
  for (;;) {
    // Získání zámku pro bezpečný přístup k proměnné
    xSemaphoreTake(mutex, portMAX_DELAY);

    // Přidání čísla "0" do společné proměnné
    sharedString += "0";

    // Výpis obsahu proměnné
    Serial.println("Text z jadra 0: " + sharedString);

    // Uvolnění zámku
    xSemaphoreGive(mutex);

    delay(2000);
  }
}

void task2(void *pvParameters) {
  for (;;) {
    // Získání zámku pro bezpečný přístup k proměnné
    xSemaphoreTake(mutex, portMAX_DELAY);

    // Přidání čísla "1" do společné proměnné
    sharedString += "1";

    // Výpis obsahu proměnné
    Serial.println("Text z jadra 1: " + sharedString);

    // Uvolnění zámku
    xSemaphoreGive(mutex);

    delay(2000);
  }
}

void setup() {
  Serial.begin(9600);

  Serial.print("Ahoj SETUP");
  Serial.print("Mutex");

  // Inicializace mutexu
  mutex = xSemaphoreCreateMutex();

  // Vytvoření úloh pro každé jádro
  xTaskCreatePinnedToCore(
      task1,    /* Funkce úlohy */
      "Task1",  /* Název úlohy */
      4096,     /* Velikost zásobníku úlohy */
      NULL,     /* Parametry úlohy */
      1,        /* Priorita úlohy */
      NULL,     /* Handler úlohy */
      0);       /* Číslo jádra (0 nebo 1) */

  xTaskCreatePinnedToCore(
      task2,    /* Funkce úlohy */
      "Task2",  /* Název úlohy */
      4096,     /* Velikost zásobníku úlohy */
      NULL,     /* Parametry úlohy */
      1,        /* Priorita úlohy */
      NULL,     /* Handler úlohy */
      1);       /* Číslo jádra (0 nebo 1) */
}

void loop() {
  // Hlavní smyčka loop() je prázdná, protože hlavní funkce úloh běží na svých jádrech.
}
