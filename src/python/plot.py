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

def read_from_file_manual(filename):
    stats = np.fromfile(STATISTICS_DIR / filename, dtype=np.float32)
    midpoint = len(stats) // 2
    return stats[midpoint:] , stats[:midpoint]

def main():
    #X, Y = read_from_file()
    X1, Y1 = read_from_file_manual("tub10k.bin")
    X2, Y2 = read_from_file_manual("tub20k.bin")
    X3, Y3 = read_from_file_manual("tub40k.bin")
    X4, Y4 = read_from_file_manual("tub80k.bin")
    X5, Y5 = read_from_file_manual("tub160k.bin")

    _fig, ax = plt.subplots(figsize=(8,5))
    ax.set_ylim([0, 0.06])

    ax.plot(X1, Y1, label="k=10k")
    ax.plot(X2, Y2, label="k=20k")
    ax.plot(X3, Y3, label="k=40k")
    ax.plot(X4, Y4, label="k=80k")
    ax.plot(X5, Y5, label="k=160k")

    ax.set_xlabel("t")
    ax.set_ylabel("density error")
    ax.legend()

    plt.show()

def main2():
    X1, Y1 = read_from_file_manual("tower400.bin")
    X2, Y2 = read_from_file_manual("tower200.bin")
    X3, Y3 = read_from_file_manual("tower100.bin")
    X4, Y4 = read_from_file_manual("tower50.bin")

    _fig, ax = plt.subplots(figsize=(8,5))
    ax.set_ylim([0, 0.06])

    ax.plot(X1, Y1, label="water column 400h")
    ax.plot(X2, Y2, label="water column 200h")
    ax.plot(X3, Y3, label="water column 100h")
    ax.plot(X4, Y4, label="water column 50h")
    ax.set_xlabel("t")
    ax.set_ylabel("density error")
    ax.legend()

    plt.show()


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        sys.exit(f"Error: {error}")
