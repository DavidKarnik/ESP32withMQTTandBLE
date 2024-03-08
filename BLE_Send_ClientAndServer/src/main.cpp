#include <Arduino.h>

// Definice UUID pro službu a charakteristiku
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

void setup() {
  // Vytvoření BLE zařízení
  BLEDevice::init("ESP32_BLE_Server");
  
  // Vytvoření BLE serveru
  pServer = BLEDevice::createServer();
  
  // Vytvoření BLE servisu
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Vytvoření BLE charakteristiky
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Vytvoření deskriptoru
  pCharacteristic->addDescriptor(new BLE2902());

  // Spuštění servisu
  pService->start();

  // Spuštění BLE serveru
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  // Posílání zprávy každých 5 sekund
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= 5000) {
    lastMillis = currentMillis;

    // Posílání "Hello, World!" přes BLE charakteristiku
    pCharacteristic->setValue("Hello, World!");
    pCharacteristic->notify();
  }

  // Nic se neděje ve smyčce
  // delay(100);
}
