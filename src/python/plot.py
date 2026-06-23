import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np

STATISTICS_DIR = Path(__file__).resolve().parents[2] / "savestate" / "statistics"


def print_files():
    print(f"Directory: {STATISTICS_DIR}:")

    files = sorted(STATISTICS_DIR.glob("*.bin"))
    if not files:
        print("  No .bin files found.")
        return

    print("  " + ", ".join(file.stem for file in files))


def read_from_file():
    print_files()
    name = input("Statistics file name: ").strip()
    filename = STATISTICS_DIR / f"{name}.bin"

    if not name:
        raise ValueError("No file was specified.")

    if not filename.is_file():
        raise FileNotFoundError(f"Cannot find file: {filename}")

    stats = np.fromfile(filename, dtype=np.float32)
    if len(stats) == 0:
        raise ValueError(f"File is empty: {filename}")
    if len(stats) % 2 != 0:
        raise ValueError(f"File has an odd number of float values: {filename}")

    midpoint = len(stats) // 2
    density_error = stats[:midpoint]
    time = stats[midpoint:]
    return time, density_error

def main():
    X, Y = read_from_file()

    _fig, ax = plt.subplots(figsize=(8,5))
    ax.plot(X, Y, label="values")

    ax.set_xlabel("t")
    ax.set_ylabel("density error")
    ax.legend()

    plt.show()


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        sys.exit(f"Error: {error}")
