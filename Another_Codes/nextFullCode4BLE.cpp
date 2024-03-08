// UART komunikace s druhým ESP32
// SPI, I2C, UART něco co posílá ... senzory, devkit, něco

// box plot .. starting ESP měření

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
// Must Add dependency to platformio.ini -> lib_deps = knolleary/PubSubClient@^2.8
#include <PubSubClient.h> // knihovna https://registry.platformio.org/libraries/knolleary/PubSubClient/installation

// BLE libraries - moc veliké knihovny... i s mqtt => málo paměti flash
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE Definice UUID pro službu a charakteristiku
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const char *filename = "/soubor.txt"; // Název souboru na SPIFFS
File fileA;                           // Proměnná pro práci se souborem

const int interruptPinCore0 = 12;
const int interruptPinCore1 = 14;

const int ledPin1 = 5;  // první LED na pin 5
const int ledPin2 = 18; // druhá LED na pin 18

// String sharedString = "";     // Společná sdílená proměnná pro obě jádra, testing
SemaphoreHandle_t mutex; // Mutex pro synchronizaci přístupu k proměnné

portMUX_TYPE muxCore0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE muxCore1 = portMUX_INITIALIZER_UNLOCKED;

volatile bool runTaskCore0 = true; // Příznak pro určení, zda má jádro 0 provádět akce
volatile bool runTaskCore1 = true; // Příznak pro určení, zda má jádro 1 provádět akce

volatile bool isLED1On = false; // Příznak pro určení, zda má LED 1 svítit
volatile bool isLED2On = false; // Příznak pro určení, zda má LED 2 svítit

const char *ssid = "ssid";
const char *password = "pass";
const char *mqtt_server = "ip";
const char *mqttTopic = "test_topic";
const int QoS = 0;

WiFiClient espClient;
PubSubClient client(espClient);

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

void IRAM_ATTR handleInterruptCore0()
{
    portENTER_CRITICAL_ISR(&muxCore0);
    runTaskCore0 = !runTaskCore0; // Přepínání příznaku pro jádro 0
    isLED1On = !isLED1On;         // Přepínání příznaku pro LED 1
    // Serial.println("Interupt jadra 0: " + runTaskCore0);
    portEXIT_CRITICAL_ISR(&muxCore0);
}

void IRAM_ATTR handleInterruptCore1()
{
    portENTER_CRITICAL_ISR(&muxCore1);
    runTaskCore1 = !runTaskCore1; // Přepínání příznaku pro jádro 1
    isLED2On = !isLED2On;         // Přepínání příznaku pro LED 2
    // Serial.println("Interupt jadra 1: " + runTaskCore1); // Doba přerušení musí být co nejkratší, seriová komunikace je moc pomalá a může způsobit problémy
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

// void writeToFile(String message)
// {
//     try
//     {
//         // Otevření souboru pro zápis (pokud neexistuje, bude vytvořen)
//         // "a" -> append, PŘIDÁVÁ na konec souboru
//         fileA = SPIFFS.open(filename, "a");
//         if (!fileA)
//         {
//             throw "Nepodarilo se otevrit soubor pro zapis.";
//         }

//         // Zápis do souboru
//         // fileA.println(message);
//         fileA.print(message);
//         fileA.close();

//         // Serial.println("\nData byla zapsana do souboru.");
//     }
//     catch (const char *error)
//     {
//         Serial.println(error);
//     }
// }

void readFile()
{
    try
    {
        File file = SPIFFS.open(filename, "r");
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
        fileA.write(message.c_str(), message.length());
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
        Serial.println(error);
        return error;
    }
}

void clearFile()
{
    try
    {
        // Otevření souboru pro smazání obsahu
        fileA = SPIFFS.open(filename, "w");
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
            BLECharacteristic::PROPERTY_NOTIFY);

    // Vytvoření deskriptoru
    pCharacteristic->addDescriptor(new BLE2902());

    // Spuštění servisu
    pService->start();

    // Spuštění BLE serveru
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void task1(void *pvParameters)
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
            xSemaphoreTake(mutex, portMAX_DELAY);

            // Přidání čísla "0" do společného souboru
            // writeToFile("0");

            // String msg = "Hello from ESP32 core 0 via MQTT";
            // client.publish("topic", getFileContext().c_str());
            client.publish("topic", writeToFileAndGetContext("0").c_str());

            // TODO -> for efficiency combine function WriteToFile and ReadFromFile to one

            // readFile();

            // Uvolnění zámku
            xSemaphoreGive(mutex);
        }
        // portEXIT_CRITICAL(&muxCore0);

        delay(2000);
    }
}

void task2(void *pvParameters)
{
    for (;;)
    {
        // delay() hlavně na jádře 1, aby se správně jádro uvedlo do provozu
        delay(100);
        digitalWrite(ledPin2, isLED2On ? HIGH : LOW);
        // portENTER_CRITICAL(&muxCore1);
        if (runTaskCore1)
        {
            xSemaphoreTake(mutex, portMAX_DELAY);

            // writeToFile("1");
            // writeToFileAndGetContext("1");

            // readFile();

            // Posílání string zpravy přes BLE charakteristiku
            // pCharacteristic->setValue(getFileContext());
            pCharacteristic->setValue(writeToFileAndGetContext("1").c_str());
            pCharacteristic->notify();

            xSemaphoreGive(mutex);
        }
        // portEXIT_CRITICAL(&muxCore1);

        delay(2000);
    }
}

void setup()
{
    Serial.begin(9600);

    Serial.println("Hello from Setup!");

    // Inicializace mutexu
    mutex = xSemaphoreCreateMutex();

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

        clearFile();

        setup_wifi();

        client.setServer(mqtt_server, 1883);
        // client.setCallback(callback); // when subscribe to topic callback is called

        setUpBLE();

        // Vytvoření úloh pro každé jádro
        xTaskCreatePinnedToCore(
            task1,   /* Funkce úlohy */
            "Task1", /* Název úlohy */
            8192,    /* Zvýšená velikost (z 4096) zásobníku úlohy (na 8 KB) */
            NULL,    /* Parametry úlohy */
            1,       /* Priorita úlohy */
            NULL,    /* Handler úlohy */
            0);      /* Číslo jádra (0 nebo 1) */

        xTaskCreatePinnedToCore(
            task2,
            "Task2",
            8192, /* Zvýšená velikost (z 4096) zásobníku úlohy (na 8 KB) */
            NULL,
            1,
            NULL,
            1);
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
