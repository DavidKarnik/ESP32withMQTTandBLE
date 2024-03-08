#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

const char *filename = "/soubor.txt"; // Název souboru na SPIFFS
File fileA; // Proměnná pro práci se souborem

void listFiles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  Serial.println("Seznam souborů v SPIFFS:");
  while (file) {
    Serial.println(" - " + String(file.name()));
    file = root.openNextFile();
  }
}

void createFile() {
  try {
    // Otevření souboru pro zápis (pokud neexistuje, bude vytvořen)
    fileA = SPIFFS.open(filename, "w");
    if (!fileA) {
      throw "Nepodařilo se otevřít soubor pro zápis.";
    }

    // Zápis do souboru
    fileA.println("Hello, ESP32!");
    fileA.close();

    Serial.println("Data byla zapsána do souboru.");
  } catch (const char *error) {
    Serial.println(error);
  }
}

void readFile() {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Nepodařilo se otevřít soubor pro čtení.");
    return;
  }

  Serial.println("Obsah souboru:");

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();
}

void setup() {
  Serial.begin(9600);

  // inicializace SPIFFS připraví paměť pro souborový systém
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS inicializován.");
    createFile();
    listFiles();
    readFile();
  } else {
    Serial.println("Nepodařilo se inicializovat SPIFFS.");
  }
}

/* void setup() {
  Serial.begin(9600);
  if (SPIFFS.format()) {
    Serial.println("SPIFFS byl úspěšně přeformátován.");
  } else {
    Serial.println("Nepodařilo se přeformátovat SPIFFS.");
  }
} */

void loop() {
  // Hlavní smyčka loop() je prázdná
}
