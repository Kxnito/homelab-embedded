import serial
import time
from collections import deque
from prometheus_client import start_http_server, Gauge

# Prometheus metrics
ax_gauge = Gauge('stm32_accel_x', 'Accelerometer X axis')
ay_gauge = Gauge('stm32_accel_y', 'Accelerometer Y axis')
az_gauge = Gauge('stm32_accel_z', 'Accelerometer Z axis')

# Rolling average filter — smooths out noise
WINDOW_SIZE = 10
ax_window = deque(maxlen=WINDOW_SIZE)
ay_window = deque(maxlen=WINDOW_SIZE)
az_window = deque(maxlen=WINDOW_SIZE)

def average(window):
    if len(window) == 0:
        return 0
    return sum(window) / len(window)

def parse_line(line):
    """Parse 'AX:123 AY:456 AZ:789' format"""
    try:
        parts = line.split()
        ax = int(parts[0].split(':')[1])
        ay = int(parts[1].split(':')[1])
        az = int(parts[2].split(':')[1])
        return ax, ay, az
    except (IndexError, ValueError):
        return None

def main():
    # Start Prometheus metrics server on port 8000
    start_http_server(8000)
    print("Prometheus metrics server started on port 8000")

    # Open serial connection to STM32
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    print(f"Connected to {ser.port}")

    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        result = parse_line(line)
        if result is None:
            continue

        ax, ay, az = result

        # Add to rolling average filter
        ax_window.append(ax)
        ay_window.append(ay)
        az_window.append(az)

        # Calculate filtered values
        ax_filtered = average(ax_window)
        ay_filtered = average(ay_window)
        az_filtered = average(az_window)

        # Update Prometheus gauges
        ax_gauge.set(ax_filtered)
        ay_gauge.set(ay_filtered)
        az_gauge.set(az_filtered)

        print(f"Raw: AX:{ax} AY:{ay} AZ:{az} | Filtered: AX:{ax_filtered:.0f} AY:{ay_filtered:.0f} AZ:{az_filtered:.0f}")

if __name__ == '__main__':
    main()