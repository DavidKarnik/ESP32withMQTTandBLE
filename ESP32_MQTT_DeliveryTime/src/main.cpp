#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <esp32-hal-misc.h>

const char *ssid = "ssid"; // SSID (WiFi name)
const char *password = "pass";     // WIFI password
const char *mqtt_server = "ip"; // my_mosquitto_broker_IP_address
const char *mqttTopic = "test_topic";
const int QoS = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// const int payloadSize = 1048576; // 1 MB payload size
// const int payloadSize = 1; // 1 byte payload size
//const size_t payloadSize = 512000; // 0.5 MB payload size
//const size_t payloadSize = 256000; // 0.25 MB payload size
const size_t payloadSize = 500; // 0.125 MB payload size
// uint8_t payloadBuffer[payloadSize];

unsigned long startTime = 0;   // Variable to store the start time
bool messageSent = false;      // Flag to indicate if the message has been sent
bool messageDelivered = false; // Flag to indicate if the message has been delivered

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  // Print the arrived message
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("], ");
  Serial.println("QoS = " + (char)QoS);

  // Calculate the time taken for message delivery
  unsigned long deliveryTime = millis() - startTime;
  Serial.print("Message delivery time: ");
  Serial.print(deliveryTime);
  Serial.println(" ms");
  messageDelivered = true; // Set the flag to true
}

void publishMessage()
{
  if (client.connected())
  {
    unsigned long currentTime = millis();
    startTime = currentTime; // Start counting elapsed time

    Serial.println("-----!-----!-----!-----!-----!-----!-----!-----");
    // Publish the blank payload with QoS level
    // Dynamically allocate memory for the payload
    uint8_t *payloadBuffer = (uint8_t *)malloc(payloadSize);
    Serial.println("Buffer of size 500 bytes created");
    if (payloadBuffer == NULL)
    {
      Serial.println("Failed to allocate memory for payload");
      return; // or handle the error accordingly
    }
    // Fill the payload buffer with zeros
    memset(payloadBuffer, 0, payloadSize);
    Serial.println("Buffer of size 500 bytes filled with zeros");

    client.publish(mqttTopic, payloadBuffer, payloadSize, QoS);
    Serial.println("Buffer of size 500 bytes published");

    // Free the allocated memory after publishing to avoid memory leaks
    free(payloadBuffer);
    Serial.println("Buffer of size 500 bytes set free");

    Serial.println("Published message to MQTT broker");
    Serial.println("-----!-----!-----!-----!-----!-----!-----!-----");
  }
  else
  {
    Serial.println("MQTT client not connected");
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
      client.subscribe(mqttTopic, 0); // client.subscribe("topic",QoS); // QoS sub 0/1 only
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

void setup()
{
  Serial.begin(115200); // Initialize Serial Monitor

  // Get the free heap memory
  size_t freeMemory = ESP.getFreeHeap();
  
  Serial.print("Free heap memory: ");
  Serial.print(freeMemory);
  Serial.println(" bytes");

  setup_wifi();
  // Connect to MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(messageReceived);
}

void loop()
{
  if (!client.connected())
  {
    reconnectMQTT();
  }
  client.loop();

  if (!messageSent)
  {
    // Publish the blank payload with QoS level
    // client.publish(mqttTopic, payloadBuffer, payloadSize, 0);
    // Set the start time
    // startTime = millis();
    publishMessage();
    messageSent = true;
  }
  // client.subscribe
  //  Check if the message has been delivered
  if (messageDelivered)
  {
    // Timeout occurred, message not delivered within 10 seconds
    Serial.println("-------------------------------------------");
    messageDelivered = true; // Set the flag to true
    // wait 10 sec and do again
    Serial.println("Wait 8 seconds and publish again");
    delay(8000);
    messageSent = false;
    messageDelivered = false;
  }
}