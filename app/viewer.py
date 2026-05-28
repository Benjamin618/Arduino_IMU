import argparse
import threading
import time
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import serial


MAX_POINTS = 400
DEFAULT_HEADER = [
    "ms",
    "ax_g",
    "ay_g",
    "az_g",
    "gx_dps",
    "gy_dps",
    "gz_dps",
    "pitch_deg",
    "roll_deg",
    "yaw_deg",
]


class DataBuffer:
    def __init__(self, max_points=MAX_POINTS):
        self.lock = threading.Lock()
        self.t = deque(maxlen=max_points)
        self.ax = deque(maxlen=max_points)
        self.ay = deque(maxlen=max_points)
        self.az = deque(maxlen=max_points)
        self.gx = deque(maxlen=max_points)
        self.gy = deque(maxlen=max_points)
        self.gz = deque(maxlen=max_points)
        self.pitch = deque(maxlen=max_points)
        self.roll = deque(maxlen=max_points)
        self.yaw = deque(maxlen=max_points)

    def add(self, row):
        with self.lock:
            self.t.append(float(row["ms"]) / 1000.0)
            self.ax.append(float(row["ax_g"]))
            self.ay.append(float(row["ay_g"]))
            self.az.append(float(row["az_g"]))
            self.gx.append(float(row["gx_dps"]))
            self.gy.append(float(row["gy_dps"]))
            self.gz.append(float(row["gz_dps"]))
            self.pitch.append(float(row["pitch_deg"]))
            self.roll.append(float(row["roll_deg"]))
            self.yaw.append(float(row["yaw_deg"]))


def serial_reader(port, baud, data_buffer):
    while True:
        try:
            with serial.Serial(port, baud, timeout=1) as ser:
                header = None
                while True:
                    line = ser.readline().decode("utf-8", errors="ignore").strip()
                    if not line:
                        continue
                    if line.startswith("INFO") or line.startswith("ERR"):
                        print(line)
                        continue
                    if line.startswith("ms,"):
                        header = line.split(",")
                        print("Header found:", header)
                        continue

                    values = line.split(",")
                    if header is None and len(values) == len(DEFAULT_HEADER):
                        # Firmware sends header once at boot. If we attach later,
                        # infer known columns from payload length.
                        header = DEFAULT_HEADER
                        print("Header inferred from payload shape.")
                    if header is None:
                        continue
                    if len(values) != len(header):
                        continue
                    row = dict(zip(header, values))
                    data_buffer.add(row)
        except serial.SerialException as exc:
            print(f"Serial error: {exc}; retry in 2s")
            time.sleep(2)


def build_plot():
    fig, axes = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
    axes[0].set_title("Raw acceleration (g)")
    axes[0].set_ylabel("g")
    axes[1].set_title("Raw gyroscope (deg/s)")
    axes[1].set_ylabel("deg/s")
    axes[2].set_title("Fused orientation (deg)")
    axes[2].set_ylabel("deg")
    axes[2].set_xlabel("Time (s)")

    acc_lines = [
        axes[0].plot([], [], label="ax")[0],
        axes[0].plot([], [], label="ay")[0],
        axes[0].plot([], [], label="az")[0],
    ]
    gyr_lines = [
        axes[1].plot([], [], label="gx")[0],
        axes[1].plot([], [], label="gy")[0],
        axes[1].plot([], [], label="gz")[0],
    ]
    ang_lines = [
        axes[2].plot([], [], label="pitch")[0],
        axes[2].plot([], [], label="roll")[0],
        axes[2].plot([], [], label="yaw")[0],
    ]

    for ax in axes:
        ax.legend(loc="upper left")
        ax.grid(True, alpha=0.3)

    return fig, axes, acc_lines, gyr_lines, ang_lines


def run_ui(data_buffer):
    fig, axes, acc_lines, gyr_lines, ang_lines = build_plot()

    def update(_frame):
        with data_buffer.lock:
            if len(data_buffer.t) < 2:
                return acc_lines + gyr_lines + ang_lines

            t = list(data_buffer.t)
            series = [
                list(data_buffer.ax),
                list(data_buffer.ay),
                list(data_buffer.az),
                list(data_buffer.gx),
                list(data_buffer.gy),
                list(data_buffer.gz),
                list(data_buffer.pitch),
                list(data_buffer.roll),
                list(data_buffer.yaw),
            ]

        for line, y in zip(acc_lines + gyr_lines + ang_lines, series):
            line.set_data(t, y)

        for axis in axes:
            axis.relim()
            axis.autoscale_view()
        axes[2].set_xlim(max(t[0], t[-1] - 20), t[-1] + 0.2)

        return acc_lines + gyr_lines + ang_lines

    anim = FuncAnimation(fig, update, interval=100, blit=False, cache_frame_data=False)
    plt.tight_layout()
    plt.show()
    return anim


def main():
    parser = argparse.ArgumentParser(description="Nano 33 BLE Sense IMU viewer")
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM5")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    args = parser.parse_args()

    data_buffer = DataBuffer()
    reader_thread = threading.Thread(
        target=serial_reader,
        args=(args.port, args.baud, data_buffer),
        daemon=True,
    )
    reader_thread.start()

    run_ui(data_buffer)


if __name__ == "__main__":
    main()
