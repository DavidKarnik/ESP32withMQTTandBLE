import asyncio
from bleak import BleakScanner, BleakClient
from bleak.exc import BleakError

async def scan_ble_devices():
    try:
        devices = await BleakScanner.discover()
        for device in devices:
            print(f"Device name: {device.name}, Address: {device.address}")
    except BleakError as e:
        print(f"An error occurred while scanning: {e}")

async def connect_to_device(address):
    try:
        async with BleakClient(address) as client:
            connected = await client.is_connected()
            if connected:
                print(f"Connected to {address}")
            else:
                print(f"Failed to connect to {address}")
    except BleakError as e:
        print(f"An error occurred while connecting: {e}")

async def main():
    await scan_ble_devices()
    # If you know the address of the device you want to connect to, use it here
    # await connect_to_device("your_device_address_here")

if __name__ == "__main__":
    asyncio.run(main())
