import asyncio
from bleak import BleakClient, BleakScanner
from datetime import datetime

BLE_DEVICE_NAME = "ESP32_BLE_Server"
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
OUTPUT_FILE = "ble_data.txt"
CONNECTION_TIMEOUT = 30.0  # Zvýšený timeout na x sekund

def notification_handler(sender, data):
    """Handle incoming data."""
    # Convert bytearray to string and strip whitespace
    data_str = data.decode('utf-8').strip()
    # Get the current timestamp
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    # Format the log entry
    log_entry = f"{timestamp} - {data_str}"
    print(log_entry)
    # Append the log entry to the file
    with open(OUTPUT_FILE, "a") as f:
        f.write(f"{log_entry}\n")

async def run():
    devices = await BleakScanner.discover()
    for device in devices:
        if device.name == BLE_DEVICE_NAME:
            try:
                async with BleakClient(device.address, timeout=CONNECTION_TIMEOUT) as client:
                    await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
                    print(f"Connected to {BLE_DEVICE_NAME}")

                    # Keep the connection open to receive notifications
                    while True:
                        await asyncio.sleep(1)
            except asyncio.TimeoutError:
                print("Connection attempt timed out.")
            except Exception as e:
                print(f"Failed to connect: {e}")

if __name__ == "__main__":
    asyncio.run(run())
