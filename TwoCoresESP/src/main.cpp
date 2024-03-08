#include <Arduino.h>

const int CORE_NUM_0 = 0;
const int CORE_NUM_1 = 1;

void task1(void *pvParameters) {
  for (;;) {
    Serial.println("Text z jadra: " + String(CORE_NUM_0));
    // String() -> potřeba nastavit větší zásobník úlohy
    delay(1000);
  }
}

void task2(void *pvParameters) {
  for (;;) {
    Serial.println("Text z jadra: " + String(CORE_NUM_1));
    delay(2000);
  }
}

void setup() {
  Serial.begin(9600);

  Serial.print("Ahoj SETUP");

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
