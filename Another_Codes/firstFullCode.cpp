#include <Arduino.h>

const int interruptPinCore0 = 12;
const int interruptPinCore1 = 14;

String sharedString = ""; // Společná sdílená proměnná pro obě jádra
SemaphoreHandle_t mutex;  // Mutex pro synchronizaci přístupu k proměnné

portMUX_TYPE muxCore0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE muxCore1 = portMUX_INITIALIZER_UNLOCKED;

volatile bool runTaskCore0 = true; // Příznak pro určení, zda má jádro 0 provádět akce
volatile bool runTaskCore1 = true; // Příznak pro určení, zda má jádro 1 provádět akce

void IRAM_ATTR handleInterruptCore0()
{
  portENTER_CRITICAL_ISR(&muxCore0);
  runTaskCore0 = !runTaskCore0; // Přepínání příznaku pro jádro 0
  // Serial.println("Interupt jadra 0: " + runTaskCore0);
  portEXIT_CRITICAL_ISR(&muxCore0);
}

void IRAM_ATTR handleInterruptCore1()
{
  portENTER_CRITICAL_ISR(&muxCore1);
  runTaskCore1 = !runTaskCore1; // Přepínání příznaku pro jádro 1
  // Serial.println("Interupt jadra 1: " + runTaskCore1); // Doba přerušení musí být co nejkratší, seriová komunikace je moc pomalá a může způsobit problémy
  portEXIT_CRITICAL_ISR(&muxCore1);
}

void task1(void *pvParameters)
{
  for (;;)
  {
    delay(100);
    // portENTER_CRITICAL(&muxCore0);
    if (runTaskCore0)
    {
      // Získání zámku pro bezpečný přístup k proměnné
      xSemaphoreTake(mutex, portMAX_DELAY);

      // Přidání čísla "0" do společné proměnné
      sharedString += "0";

      Serial.println("Text z jadra 0: " + sharedString);

      // Uvolnění zámku
      xSemaphoreGive(mutex);
    }
    // portEXIT_CRITICAL(&muxCore0);

    delay(1000);
  }
}

void task2(void *pvParameters)
{
  for (;;)
  {
    // delay hlavně na jádře 1, aby se správně jádro uvedlo do provozu
    delay(100);
    // Serial.println("Jsem v úloze 1 FOR CYKLUS");
    // portENTER_CRITICAL(&muxCore1);
    if (runTaskCore1)
    {
      // Serial.println("Jsem v úloze 1");

      xSemaphoreTake(mutex, portMAX_DELAY);

      sharedString += "1";

      Serial.println("Text z jadra 1: " + sharedString);

      xSemaphoreGive(mutex);
    }
    // portEXIT_CRITICAL(&muxCore1);

    delay(1000);
  }
}

void setup()
{
  Serial.begin(9600);

  Serial.println("Ahoj SetUp !");

  // Inicializace mutexu
  mutex = xSemaphoreCreateMutex();

  // Nastavení pinů pro přerušení jádra 0 a jádra 1
  pinMode(interruptPinCore0, INPUT_PULLUP);
  pinMode(interruptPinCore1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinCore0), handleInterruptCore0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinCore1), handleInterruptCore1, CHANGE);

  // Vytvoření úloh pro každé jádro
  xTaskCreatePinnedToCore(
      task1,   /* Funkce úlohy */
      "Task1", /* Název úlohy */
      4096,    /* Velikost zásobníku úlohy */
      NULL,    /* Parametry úlohy */
      1,       /* Priorita úlohy */
      NULL,    /* Handler úlohy */
      0);      /* Číslo jádra (0 nebo 1) */

  xTaskCreatePinnedToCore(
      task2,   /* Funkce úlohy */
      "Task2", /* Název úlohy */
      8192,    /* Zvýšená velikost zásobníku úlohy (8 KB) */
      NULL,    /* Parametry úlohy */
      1,       /* Priorita úlohy */
      NULL,    /* Handler úlohy */
      1);      /* Číslo jádra (0 nebo 1) */
}

void loop()
{
  // Hlavní smyčka loop() je prázdná, protože hlavní funkce úloh běží na svých jádrech.
}
