import paho.mqtt.client as mqtt
import random
import time

# MQTT settings
MQTT_BROKER = "localhost"  # You can use your own broker here
MQTT_PORT = 1883
MQTT_TOPIC = "factory/machine/vibration"

# Function to simulate vibration sensor data
def simulate_vibration():
    return random.uniform(0, 1)  # Random vibration value between 0 and 1

# The callback for when the client receives a CONNACK response from the server
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")

# The callback for when a PUBLISH message is sent to the server
def on_publish(client, userdata, mid):
    print(f"Message {mid} published")

# Create an MQTT client instance
client = mqtt.Client()

# Assign event callbacks
client.on_connect = on_connect
client.on_publish = on_publish

# Connect to the broker
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start the loop
client.loop_start()

try:
    while True:
        # Simulate vibration sensor data
        vibration_value = round(simulate_vibration(),3)

        # Publish the simulated data
        client.publish(MQTT_TOPIC, vibration_value)

        # Print the published value
        print(f"Published: {vibration_value} to topic {MQTT_TOPIC}")

        # Wait for 2 seconds before sending the next data
        time.sleep(2)

except KeyboardInterrupt:
    print("Simulation stopped")

# Stop the loop
client.loop_stop()

# Disconnect from the broker
client.disconnect()
