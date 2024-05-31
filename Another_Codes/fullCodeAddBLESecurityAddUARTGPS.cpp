#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
// Must Add dependency to platformio.ini -> lib_deps = knolleary/PubSubClient@^2.8
#include <PubSubClient.h> // knihovna https://registry.platformio.org/libraries/knolleary/PubSubClient/installation

// BLE libraries - moc veliké knihovny... i s mqtt => málo paměti flash
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
// náhrada:
#include <ArduinoBLE.h>

// BLE Definice UUID pro službu a charakteristiku
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEService myService(SERVICE_UUID);
// 20 b -> maximální délka zprávy
BLEStringCharacteristic myCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 20);
// Deklarace proměnné staticPIN
uint32_t staticPIN = 123456;

// Advertising parameters (info about advertiser)
// const uint8_t manufactData[4] = {0x01, 0x02, 0x03, 0x04};
// const uint8_t serviceData[3] = {0x00, 0x01, 0x02};

const char *filename = "/soubor.txt"; // Název sdíleného souboru na SPIFFS
File fileA;                           // Proměnná pro práci se souborem

const int interruptPinCore0 = 12;
const int interruptPinCore1 = 14;

const int ledPin1 = 13; // první LED na pin 5
const int ledPin2 = 27; // druhá LED na pin 18

// String sharedString = "";     // Společná sdílená proměnná pro obě jádra, testing
// SemaphoreHandle_t mutex; // Mutex pro synchronizaci přístupu k sdílené proměnné, souboru
std::mutex fileMutex;

portMUX_TYPE muxCore0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE muxCore1 = portMUX_INITIALIZER_UNLOCKED;

volatile bool runTaskCore0 = true; // Příznak pro určení, zda má jádro 0 provádět akce
volatile bool runTaskCore1 = true; // Příznak pro určení, zda má jádro 1 provádět akce

volatile bool isLED1On = false; // Příznak pro určení, zda má LED 1 svítit
volatile bool isLED2On = false; // Příznak pro určení, zda má LED 2 svítit

const char *ssid = "KKK";          // SSID (WiFi name)
const char *password = "adrianka"; // WIFI password
// my_mosquitto_broker_IP_address
const char *mqtt_server = "ip";
const char *mqttTopic = "topic";
const int QoS = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR handleInterruptCore0()
{
  portENTER_CRITICAL_ISR(&muxCore0);
  runTaskCore0 = !runTaskCore0; // Přepínání příznaku pro jádro 0
  isLED1On = !isLED1On;         // Přepínání příznaku pro LED 1
  // Doba přerušení musí být co nejkratší, seriová komunikace je moc pomalá a může způsobit problémy
  // Serial.println("Interupt jadra 0: " + runTaskCore0);
  portEXIT_CRITICAL_ISR(&muxCore0);
}

void IRAM_ATTR handleInterruptCore1()
{
  portENTER_CRITICAL_ISR(&muxCore1);
  runTaskCore1 = !runTaskCore1;
  isLED2On = !isLED2On;
  // Serial.println("Interupt jadra 1: " + runTaskCore1);
  portEXIT_CRITICAL_ISR(&muxCore1);
}

// void listFiles()
// {
//     try
//     {
//         File root = SPIFFS.open("/");
//         if (!root)
//         {
//             throw "Nepodarilo se otevrit root adresar.";
//         }

//         File file = root.openNextFile();

//         Serial.println("Seznam souboru v SPIFFS:");
//         while (file)
//         {
//             Serial.println(" - " + String(file.name()));
//             file = root.openNextFile();
//         }
//     }
//     catch (const char *error)
//     {
//         Serial.println(error);
//     }
// }

void writeToFile(String message)
{
  try
  {
    // Otevření souboru pro zápis (pokud neexistuje, bude vytvořen)
    // FILE_WRITE - zápis do zadaného souboru (tím se přepíše obsah, který se již v souboru nachází)
    fileA = SPIFFS.open(filename, FILE_WRITE);
    if (!fileA)
    {
      throw "Nepodarilo se otevrit soubor pro zapis.";
    }

    // Zápis do souboru
    // fileA.println(message);
    fileA.print(message);
    fileA.close();

    // Serial.println("\nData byla zapsana do souboru.");
  }
  catch (const char *error)
  {
    Serial.println(error);
  }
}

void readFile()
{
  try
  {
    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
      throw "Nepodarilo se otevrit soubor pro cteni.";
    }

    // Serial.println("\nObsah souboru:");

    while (file.available())
    {
      Serial.write(file.read());
    }
    Serial.println();

    file.close();
  }
  catch (const char *error)
  {
    Serial.println(error);
  }
}

// String getFileContext()
// {
//     String out = "";
//     try
//     {
//         File file = SPIFFS.open(filename, "r");
//         if (!file)
//         {
//             throw "Nepodarilo se otevrit soubor pro cteni.";
//         }

//         // Serial.println("\nObsah souboru:");

//         while (file.available())
//         {
//             out = out + file.read();
//         }

//         file.close();

//         return out;
//     }
//     catch (const char *error)
//     {
//         // throw error;
//         return error;
//         // Serial.println(error);
//     }
// }

String writeToFileAndGetContext(String message)
{
  String out = "";
  try
  {
    // Otevření souboru pro zápis na konec a čtení
    fileA = SPIFFS.open(filename, "a+");
    if (!fileA)
    {
      throw "Nepodařilo se otevřít soubor.";
    }

    // Přidání zprávy do souboru
    fileA.write((const uint8_t *)message.c_str(), message.length());
    fileA.seek(0); // Přesun na začátek souboru pro čtení

    // Čtení obsahu souboru
    while (fileA.available())
    {
      out = out + char(fileA.read());
    }

    // Uzavření souboru
    fileA.close();

    // Vrácení obsahu souboru
    return out;
  }
  catch (const char *error)
  {
    // Serial.println(error);
    return error;
  }
}

void clearFile(const char *_filename)
{
  try
  {
    // Otevření souboru pro smazání obsahu
    fileA = SPIFFS.open(_filename, "w");
    if (!fileA)
    {
      throw "Nepodarilo se otevrit soubor pro smazani obsahu.";
    }

    // Zavření souboru (smazání obsahu)
    fileA.close();

    Serial.println("\nObsah souboru byl vymazan.");
  }
  catch (const char *error)
  {
    Serial.println(error);
    // throw error;
  }
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  delay(10);
  WiFi.begin(ssid, password);
  delay(100);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      // client.subscribe(mqttTopic, QoS); // QoS when sub 0/1 only
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void setUpBLE()
{
  if (!BLE.begin())
  {
    Serial.println("failed to initialize BLE!");
    return;
    // throw "Nepodarilo se inicializovat BLE.";
  }

  // myService.addCharacteristic(myCharacteristic);
  // BLE.addService(myService);

  // Build scan response data packet
  // BLEAdvertisingData scanData;

  // Set parameters for scan response packet
  // scanData.setLocalName("Test enhanced advertising");

  // Copy set parameters in the actual scan response packet
  // BLE.setScanResponseData(scanData);

  // Build advertising data packet
  // BLEAdvertisingData advData;

  // Set parameters for advertising packet
  // advData.setManufacturerData(0x004C, manufactData, sizeof(manufactData));
  // advData.setAdvertisedService(myService);

  // advData.setAdvertisedServiceData(0xfff0, serviceData, sizeof(serviceData));
  // Copy set parameters in the actual advertising packet
  // BLE.setAdvertisingData(advData);

  // SETUP BLE ADVERTISING DATA
  BLE.setLocalName("ESP32_BLE_Server");
  BLE.setAdvertisedService(myService);
  myService.addCharacteristic(myCharacteristic);
  BLE.addService(myService);

  BLE.advertise();

  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setStaticPIN(staticPIN);

  Serial.println("BLE advertising ...");
}

void task1MQTTPublish(void *pvParameters)
{
  for (;;)
  {
    digitalWrite(ledPin1, isLED1On ? HIGH : LOW);

    if (!client.connected())
    {
      reconnectMQTT();
    }
    client.loop();
    // client.publish(mqttTopic, payloadBuffer, payloadSize, QoS);

    // portENTER_CRITICAL(&muxCore0);
    if (runTaskCore0)
    {
      // Získání zámku pro bezpečný přístup k proměnné
      // Zámek by měl trvat co nejkratší dobu !
      // xSemaphoreTake(mutex, portMAX_DELAY);

      // Přidání čísla "0" do společného souboru
      // writeToFile("0");

      // String msg = "Hello from ESP32 core 0 via MQTT";
      client.publish(mqttTopic, getFileContext().c_str());
      // client.publish("topic", writeToFileAndGetContext("0").c_str());

      // readFile();

      // Uvolnění zámku
      // xSemaphoreGive(mutex);
    }
    // portEXIT_CRITICAL(&muxCore0);

    delay(1000);
  }
}

void task2BLEadvertise(void *pvParameters)
{
  for (;;)
  {
    BLEDevice central = BLE.central();
    // Is some device connected to BLE server?
    if (central)
    {
      while (central.connected())
      {
        // delay() hlavně na jádře 1, aby se správně jádro uvedlo do provozu
        delay(100);
        digitalWrite(ledPin2, isLED2On ? HIGH : LOW);
        // portENTER_CRITICAL(&muxCore1);
        if (runTaskCore1)
        {
          // Serial.print("Connected to central: ");
          // Serial.println(BLE.central().address());
          // xSemaphoreTake(mutex, portMAX_DELAY);

          // myCharacteristic.writeValue("Hello from ESP32");
          myCharacteristic.writeValue(getFileContext().c_str());

          // xSemaphoreGive(mutex);
          //   xSemaphoreTake(mutex, portMAX_DELAY);

          //   // writeToFile("1");
          //   // writeToFileAndGetContext("1");

          //   // readFile();

          //   // Posílání string zpravy přes BLE charakteristiku
          //   myCharacteristic.writeValue("Hello from ESP32");

          //   xSemaphoreGive(mutex);
        }
        delay(1000);
      }
      // portEXIT_CRITICAL(&muxCore1);
    }
    delay(100);
  }
}

void task3UARTcollect(void *pvParameters)
{
  String receivedData = ""; // Initialize a string to store the received data
  for (;;)
  {
    if (SerialUART.available())
    { // If there is data available to read from UART
      receivedData = "";
      while (SerialUART.available())
      {                              // Read all available data from UART1
        char ch = SerialUART.read(); // Read one character
        receivedData += ch;          // Add the character to the receivedData string
      }
      // Serial.println(receivedData); // Print the received data to the serial monitor

      // xSemaphoreTake(mutex, portMAX_DELAY);
      // Write received data to the file
      // dataFile.println(receivedData);
      std::lock_guard<std::mutex> lock(fileMutex); // Acquire mutex lock before writing to the file
      writeToFile(receivedData);

      // xSemaphoreGive(mutex);
    }
  }
  vTaskDelay(10 / portTICK_PERIOD_MS); // Delay to reduce CPU load ( 10 ms )
}

void setup()
{
  Serial.begin(9600);

  SerialUART.begin(9600, SERIAL_8N1, 25, 26); // Initialize UART with custom pins, 8N1 - data frame nastavení

  Serial.println("Hello from Setup!");

  // Inicializace mutexu
  // mutex = xSemaphoreCreateMutex();

  // Nastavení pinů pro přerušení jádra 0 a jádra 1
  pinMode(interruptPinCore0, INPUT_PULLUP);
  pinMode(interruptPinCore1, INPUT_PULLUP);

  // Nastavení pinů pro LED diody
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);

  // Nastavení přerušení pro obě jádra
  attachInterrupt(digitalPinToInterrupt(interruptPinCore0), handleInterruptCore0, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinCore1), handleInterruptCore1, FALLING);

  // inicializace SPIFFS připraví paměť pro souborový systém
  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS inicializovan.");
    // listFiles();

    clearFile(filename);

    setup_wifi();

    client.setServer(mqtt_server, 1883);
    // client.setCallback(callback); // when subscribe to topic callback is called

    setUpBLE();

    // Vytvoření úloh pro každé jádro
    // Core 0 -----------------------------------------------------------------
    xTaskCreatePinnedToCore(
        task1MQTTPublish,   /* Funkce úlohy */
        "Task1MQTTPublish", /* Název úlohy */
        8192,               /* Zvýšená velikost (z 4096) zásobníku úlohy (na 8 KB) */
        NULL,               /* Parametry úlohy */
        1,                  /* Priorita úlohy */
        NULL,               /* Handler úlohy */
        0);                 /* Číslo jádra (0 nebo 1) */
    // Multithreading ->
    xTaskCreatePinnedToCore(task3UARTcollect, "Task3UARTcollect", 8192, NULL, 1, NULL, 0);

    // Core 1 -----------------------------------------------------------------
    xTaskCreatePinnedToCore(task2BLEadvertise, "Task2BLEadvertise", 8192, NULL, 1, NULL, 1);
  }
  else
  {
    Serial.println("Nepodarilo se inicializovat SPIFFS.");
  }
}

void loop()
{
  // Hlavní smyčka loop() je prázdná, protože hlavní funkce úloh běží na svých jádrech.
}
