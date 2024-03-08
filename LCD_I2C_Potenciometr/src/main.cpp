#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// I2C adresa LCD monitoru (najděte správnou adresu pro váš LCD monitor)
#define LCD_ADDR 0x27

// Vytvoření instance LiquidCrystal_I2C s I2C adresou a velikostí displeje
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// LCD I2C piny Analog: pro SDA pin 21 a SCL pin 22
// Pin pro připojení potenciometru (Prostřední)
// levý pin VCC +5V, pravý pin GND, nebo obráceně
const int potPin = 34; // Přizpůsobte tomuto pinu podle vašeho zapojení

void setup() {
  // Inicializace I2C komunikace a LCD monitoru
  Wire.begin();
  lcd.begin(16, 2);

  // Inicializace pinu pro potenciometr
  pinMode(potPin, INPUT);
}

void loop() {
  // Čtení hodnoty potenciometru
  int potValue = analogRead(potPin);

  // Nastavení kontrastu LCD monitoru podle hodnoty potenciometru
  lcd.setContrast(potValue);

  // Zobrazení hodnoty potenciometru na LCD monitoru
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pot Value: ");
  lcd.print(potValue);

  delay(100); // Zpoždění pro stabilitu
}
