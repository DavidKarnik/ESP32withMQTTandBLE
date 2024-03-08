#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "name";
const char* password = "pass";
const char* mqtt_server = "ip";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  delay(100);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("topic",2);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  String msg = "Hello from ESP32";
  client.publish("topic", msg.c_str());
  delay(5000);
}