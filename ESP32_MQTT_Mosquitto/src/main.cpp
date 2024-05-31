#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h> // knihovna https://registry.platformio.org/libraries/knolleary/PubSubClient/installation

const char *ssid = "ssid";         // SSID (WiFi name)
const char *password = "password"; // WIFI password
// my_mosquitto_broker_IP_address
const char *mqtt_server = "ip";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //WiFi.begin(ssid, password);
  //delay(1000);
  //WiFi.disconnect();
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
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("topic",2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  String msg = "Hello from ESP32";
  client.publish("topic", msg.c_str());
  // Publish a message with QoS level 1
  //client.publish("test_topic", "Hello, World!", true, 1);
  delay(5000);
}