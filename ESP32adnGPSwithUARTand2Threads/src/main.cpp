#include <Arduino.h>
#include <SPIFFS.h>
#include <mutex>

File dataFile; // File object for storing data
// std::mutex fileMutex; // Mutex for file access
SemaphoreHandle_t mutex; // Mutex pro synchronizaci přístupu k proměnné

String getFileContext()
{
  String out = "";
  try
  {
    dataFile = SPIFFS.open("/data.txt", FILE_READ);
    if (!dataFile)
    {
      throw "Nepodarilo se otevrit soubor pro cteni.";
    }

    // Serial.println("\nObsah souboru:");
    dataFile.seek(0); // Move the file pointer to the beginning of the file
    while (dataFile.available())
    {
      out = out + dataFile.read();
    }
    dataFile.close();

    return out;
  }
  catch (const char *error)
  {
    // throw error;
    return error;
    // Serial.println(error);
  }
}
String readFileContent() {
    String content = "";
    xSemaphoreTake(mutex, portMAX_DELAY);

    dataFile = SPIFFS.open("/data.txt", FILE_READ);
    if (dataFile) {
        dataFile.seek(0); // Move the file pointer to the beginning of the file
        while (dataFile.available()) {
            char ch = dataFile.read(); // Read one character
            content += ch;             // Append character to content
        }
        dataFile.close(); // Close the file
    }
    xSemaphoreGive(mutex);

    return content;
}

void readUartAndSaveToFile(void *pvParameters)
{
    TickType_t lastExecutionTime = xTaskGetTickCount(); // Uložení času posledního spuštění úlohy

    while (true)
    {
        // Čtení dat z UARTu a ukládání do souboru
        if (Serial1.available())
        {                             // Pokud jsou k dispozici data k přijetí na UART1
            String receivedData = ""; // Inicializace řetězce pro ukládání přijatých dat
            while (Serial1.available())
            {                             // Čtení všech dostupných dat z UART1
                char ch = Serial1.read(); // Čtení jednoho znaku
                receivedData += ch;       // Přidání znaku do řetězce receivedData
            }
            // Serial.println(receivedData); // Vypsání přijatých dat do sériového monitoru

            // std::lock_guard<std::mutex> lock(fileMutex); // Acquire mutex lock before writing to the file
            xSemaphoreTake(mutex, portMAX_DELAY);

            dataFile = SPIFFS.open("/data.txt", FILE_WRITE);

            dataFile.println(receivedData); // Write received data to the file
            dataFile.close();               // Close the file
            xSemaphoreGive(mutex);
        }

        // Kontrola času a výpis souboru na sériový monitor jednou za 5 sekund
        if (xTaskGetTickCount() - lastExecutionTime >= pdMS_TO_TICKS(2000))
        {
            lastExecutionTime = xTaskGetTickCount(); // Aktualizace času posledního spuštění úlohy
                                                     // Výpis obsahu souboru na sériový monitor
                                                     // std::lock_guard<std::mutex> lock(fileMutex); // Acquire mutex lock before reading from the file
            // xSemaphoreTake(mutex, portMAX_DELAY);

            // dataFile = SPIFFS.open("/data.txt", FILE_READ);

            // dataFile.seek(0); // Move the file pointer to the beginning of the file
            // while (dataFile.available())
            // {                              // Read all available data from the file
            //     char ch = dataFile.read(); // Read one character
            //     Serial.print(ch);          // Print the character to the serial monitor
            // }
            // dataFile.close(); // Close the file

            // Serial.println("getFileContext(): --------------------------------------");
            // String getF = getFileContext();

            Serial.println("readFileContent(): --------------------------------------");
            String getF = readFileContent();

            Serial.println(getF);

            // xSemaphoreGive(mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Zpoždění 1 ms
    }
}

void setup()
{
    Serial.begin(9600);                      // Initialize serial communication for the serial monitor
    Serial1.begin(9600, SERIAL_8N1, 25, 26); // Initialize UART1 with custom pins

    if (!SPIFFS.begin())
    { // Initialize SPIFFS
        Serial.println("Failed to mount file system");
        return;
    }

    // dataFile = SPIFFS.open("/data.txt", FILE_APPEND); // Open the data file in append mode
    // if (!dataFile)
    // {
    //     Serial.println("Failed to open data file");
    //     return;
    // }

    mutex = xSemaphoreCreateMutex(); // Create a mutex semaphore
    if (mutex == NULL)
    {
        Serial.println("Failed to create mutex");
        return;
    }

    xTaskCreatePinnedToCore(readUartAndSaveToFile, "ReadUART", 10000, NULL, 1, NULL, 0); // Create a task for reading UART data and saving to file
}

void loop()
{
    // Empty loop
}
