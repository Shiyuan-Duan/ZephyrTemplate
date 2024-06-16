
address = "EEB84381-DEE3-2535-3295-0632CF38680F"


import matplotlib.pyplot as plt
import asyncio
from bleak import BleakClient
import pandas as pd
from scipy.signal import butter, sosfilt
# Constants
UPDATE_INTERVAL = 0.002  # Notification frequency in seconds
DATA_POINTS = int(5 / UPDATE_INTERVAL)  # Data points for 5 seconds

# Set up plot
plt.ion()
fig, ax = plt.subplots()
line, = ax.plot([], [])
plt.ylabel('Data Values')
plt.xlabel('Time')
plt.title('Real-Time BLE Data (Last 5 seconds)')

data_list = []
data_list_all = []
def update_plot(new_data):
    data_list.extend(new_data)

    # print(len(df))
    # Keep only the last DATA_POINTS of data
    data_list[:] = data_list[-DATA_POINTS:]
    line.set_data(range(len(data_list)), data_list)
    ax.relim()
    ax.autoscale_view(True, True, True)
    fig.canvas.draw()
    fig.canvas.flush_events()


notify_uuid = "000062c7-b99e-4141-9439-c4f9db977899"  # Notification UUID
def butter_bandstop_filter(data, lowcut, highcut, fs, order=2):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    sos = butter(order, [low, high], btype='bandstop', output='sos')
    y = sosfilt(sos, data)
    return y
def notification_handler(sender, data):
    int_size = 4
    print(data)
    print(data[0])
    # Convert the entire byte array into a list of int32_t
    new_ints = [int.from_bytes(data[i:i+int_size], byteorder='little', signed=True)
                for i in range(0, len(data), int_size)]
    
    # Reverse the list to drop zeros from the end (now beginning)
    reversed_ints = list(reversed(new_ints))
    
    # Drop zeros until the first non-zero element is found
    while reversed_ints and reversed_ints[0] == 0:
        reversed_ints.pop(0)
    
    # Reverse the list back to original order
    final_ints = list(reversed(reversed_ints))
    fs = 512  # Sample rate, Hz
    lowcut = 49.5  # Lower boundary of the stop band, Hz
    highcut = 50.5  # Upper boundary of the stop band, Hz
    
    # Apply band-stop filter
    filtered_ints = butter_bandstop_filter(final_ints, lowcut, highcut, fs, order=2)
    
    update_plot(filtered_ints)

async def connect_and_notify(address, uuid):
    async with BleakClient(address) as client:
        if await client.connect():
            print("Connected successfully!")
            await client.start_notify(uuid, notification_handler)
            await asyncio.sleep(600)  # Keep connection for 10 minutes
            await client.stop_notify(uuid)
        else:
            print("Failed to connect.")

# Run the event loop
loop = asyncio.get_event_loop()
loop.run_until_complete(connect_and_notify(address, notify_uuid))
