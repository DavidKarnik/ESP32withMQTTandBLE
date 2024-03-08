#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "ssid";
const char* password = "pass";
// my_mosquitto_broker_IP_address
const char* mqtt_server = "ip";
const int buttonPin = 2; // Pin, na kterém je připojeno tlačítko
const int ledPin = 15;   // Pin, na kterém je připojena LED dioda

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  delay(10);
  WiFi.begin(ssid, password);
  delay(100);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("topic", 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishMessage(void * parameter) {
  for (;;) {
    // Přečtěte stav tlačítka
    int buttonState = digitalRead(buttonPin);

    // Pokud je tlačítko stisknuto, rozsviťte LED a pošlete zprávu na MQTT
    if (buttonState == HIGH) {
      digitalWrite(ledPin, HIGH);
      String msg = "Hello from ESP32";
      client.publish("topic", msg.c_str());
      Serial.println("Message published");
      delay(1000); // Zpoždění pro zabránění opakovaného odesílání
    } else {
      digitalWrite(ledPin, LOW);
    }

    delay(10); // Drobné zpoždění pro stabilitu
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  xTaskCreatePinnedToCore(
    publishMessage, // Funkce, kterou chcete spustit na jádře
    "publishMessage", // Název úlohy
    4096, // Velikost zásobníku úlohy
    NULL, // Parametr pro úlohu (NULL, protože nemáme žádné parametry)
    1, // Priorita úlohy
    NULL, // Handle pro úlohu (NULL, protože nepotřebujeme získat handle)
    0 // Jádro, na kterém má běžet úloha (0 pro jádro 0)
  );
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
