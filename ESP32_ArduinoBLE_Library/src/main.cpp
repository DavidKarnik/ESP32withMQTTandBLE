#include <Arduino.h>
// BLE libraries - moc veliké knihovny... i s mqtt => málo paměti flash
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
// náhrada:
#include <ArduinoBLE.h>
#define LED_BUILTIN 13

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEService customService(SERVICE_UUID);
BLEStringCharacteristic customCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 20);

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }

  BLE.setLocalName("ESP32_BLE_Server");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(customCharacteristic);
  BLE.addService(customService);

  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop()
{
  BLEDevice central = BLE.central();

  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected())
    {
      // Posílání řetězce "Hello from ESP32"
      customCharacteristic.writeValue("Hello from ESP32");
      delay(200);
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}
