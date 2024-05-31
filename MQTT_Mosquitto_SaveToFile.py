import paho.mqtt.client as mqtt
from datetime import datetime

def on_message(client, userdata, message):
    # Get the current timestamp
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    # Decode the message payload and remove any extraneous newline characters
    payload = message.payload.decode('utf-8').strip()
    # Format the log entry
    log_entry = f"{current_time} - {payload}"
    # Print the log entry to the console
    print(log_entry)
    # Write the log entry to the file
    with open("mqtt_data.txt", "a") as file:
        file.write(f"{log_entry}\n")

# Initialize the MQTT client
client = mqtt.Client()

# Define the connection details
broker_address = "ip"
broker_port = 1883
keepalive_interval = 60
topic = "topic"

# Set the on_message callback
client.on_message = on_message

# Connect to the MQTT broker
client.connect(broker_address, broker_port, keepalive_interval)

# Subscribe to the topic
client.subscribe(topic)

# Start the loop to process received messages
client.loop_forever()
