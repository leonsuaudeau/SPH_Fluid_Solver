import sys
from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import numpy as np

STATISTICS_DIR = Path(__file__).resolve().parents[2] / "savestate" / "statistics"

"""
Disclaimer: OpenAI Codex was used to make plots more visually appealing
"""

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
    return stats[midpoint:], stats[:midpoint]
def main():
    # X, Y = read_from_file()
    X1, Y1 = read_from_file_manual("tub10k.bin")
    X2, Y2 = read_from_file_manual("tub20k.bin")
    X3, Y3 = read_from_file_manual("tub40k.bin")
    X4, Y4 = read_from_file_manual("tub80k.bin")
    X5, Y5 = read_from_file_manual("tub160k.bin")

    series = (
        (X1, Y1, r"$10\,000$"),
        (X2, Y2, r"$20\,000$"),
        (X3, Y3, r"$40\,000$"),
        (X4, Y4, r"$80\,000$"),
        (X5, Y5, r"$160\,000$"),
    )
    colors = ("#0072B2", "#E69F00", "#009E73", "#D55E00", "#CC79A7")
    line_styles = ("-", "--", "-.", ":", (0, (5, 1)))

    fig, ax = plt.subplots(figsize=(10, 3.2), dpi=150, constrained_layout=True)

    for (time, density_error, label), color, line_style in zip(
        series, colors, line_styles
    ):
        ax.plot(
            time,
            100.0 * density_error,
            color=color,
            linestyle=line_style,
            linewidth=1.6,
            label=label,
        )

    ax.set(xlabel=r"Time, $t$", ylabel="Density error (%)", ylim=(0.0, 6.0))
    ax.margins(x=0)
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)

    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(
        title=r"$k$",
        loc="upper center",
        ncol=len(series),
        frameon=False,
        columnspacing=1.5,
        handlelength=2.5,
    )

    plt.show()
def main_density_error_comparison():
    datasets = (
        (10_000, *read_from_file_manual("tub10k.bin")),
        (20_000, *read_from_file_manual("tub20k.bin")),
        (40_000, *read_from_file_manual("tub40k.bin")),
        (80_000, *read_from_file_manual("tub80k.bin")),
        (160_000, *read_from_file_manual("tub160k.bin")),
    )
    colors = ("#0072B2", "#E69F00", "#009E73", "#D55E00", "#CC79A7")
    line_styles = ("-", "--", "-.", ":", (0, (5, 1)))

    fig, (ax_time, ax_mean) = plt.subplots(
        1,
        2,
        figsize=(10, 3.2),
        dpi=150,
        constrained_layout=True,
        gridspec_kw={"width_ratios": (3, 2)},
    )

    mean_errors = []
    for (k, time, density_error), color, line_style in zip(
        datasets, colors, line_styles
    ):
        density_error_percent = 100.0 * density_error
        mean_errors.append(np.mean(density_error_percent))
        ax_time.plot(
            time,
            density_error_percent,
            color=color,
            linestyle=line_style,
            linewidth=1.6,
            label=rf"${k:,}$".replace(",", r"\,"),
        )

    ax_time.set(
        xlabel=r"Time, $t$", ylabel="Density error (%)", ylim=(0.0, 6.0)
    )
    ax_time.margins(x=0)
    ax_time.legend(
        title=r"$k$",
        loc="upper center",
        ncol=len(datasets),
        frameon=False,
        fontsize=8,
        title_fontsize=9,
        columnspacing=0.9,
        handlelength=1.8,
        handletextpad=0.4,
    )

    k_values = np.array([dataset[0] for dataset in datasets])
    ax_mean.plot(k_values, mean_errors, color="#666666", linewidth=1.2, zorder=1)
    ax_mean.scatter(
        k_values,
        mean_errors,
        c=colors,
        s=42,
        edgecolors="white",
        linewidths=0.8,
        zorder=2,
    )
    ax_mean.set_xscale("log", base=2)
    ax_mean.set_yscale("log", base=2)
    ax_mean.set_xticks(k_values, [f"{k // 1_000}k" for k in k_values])
    ax_mean.set_yticks(
        (0.25, 0.5, 1.0, 2.0, 4.0), ("0.25", "0.5", "1", "2", "4")
    )
    ax_mean.minorticks_off()
    ax_mean.set(xlabel=r"Stiffness, $k$", ylabel="Mean density error (%)")

    for ax in (ax_time, ax_mean):
        ax.set_axisbelow(True)
        ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
        ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
        ax.spines[["top", "right"]].set_visible(False)
        ax.spines[["left", "bottom"]].set_color("#666666")
        ax.tick_params(colors="#333333", direction="out", length=3)

    plt.show()
def main_water_column():
    datasets = (
        (50, *read_from_file_manual("tower50.bin")),
        (100, *read_from_file_manual("tower100.bin")),
        (200, *read_from_file_manual("tower200.bin")),
        (400, *read_from_file_manual("tower400.bin")),
    )
    colors = ("#0072B2", "#E69F00", "#009E73", "#D55E00")
    line_styles = ("-", "--", "-.", ":")

    fig, (ax_time, ax_mean) = plt.subplots(
        1,
        2,
        figsize=(10, 3.2),
        dpi=150,
        constrained_layout=True,
        gridspec_kw={"width_ratios": (3, 2)},
    )

    mean_errors = []
    for (height, time, density_error), color, line_style in zip(
        datasets, colors, line_styles
    ):
        density_error_percent = 100.0 * density_error
        mean_errors.append(np.mean(density_error_percent))
        ax_time.plot(
            time,
            density_error_percent,
            color=color,
            linestyle=line_style,
            linewidth=1.6,
            label=rf"${height}h$",
        )

    ax_time.set(
        xlabel=r"Time, $t$", ylabel="Density error (%)", ylim=(0.0, 6.0)
    )
    ax_time.margins(x=0)
    ax_time.legend(
        title="Water-column height",
        loc="center",
        bbox_to_anchor=(0.5, 0.64),
        ncol=len(datasets),
        frameon=False,
        fontsize=8,
        title_fontsize=9,
        columnspacing=0.9,
        handlelength=1.8,
        handletextpad=0.4,
    )

    heights = np.array([dataset[0] for dataset in datasets])
    ax_mean.plot(heights, mean_errors, color="#666666", linewidth=1.2, zorder=1)
    ax_mean.scatter(
        heights,
        mean_errors,
        c=colors,
        s=42,
        edgecolors="white",
        linewidths=0.8,
        zorder=2,
    )
    ax_mean.set_xscale("log", base=2)
    ax_mean.set_yscale("log", base=2)
    ax_mean.set_xticks(heights, [str(height) for height in heights])
    ax_mean.set_yticks((0.5, 1.0, 2.0, 4.0), ("0.5", "1", "2", "4"))
    ax_mean.minorticks_off()
    ax_mean.set(
        xlabel=r"Water-column height, $H/h$", ylabel="Mean density error (%)"
    )

    for ax in (ax_time, ax_mean):
        ax.set_axisbelow(True)
        ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
        ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
        ax.spines[["top", "right"]].set_visible(False)
        ax.spines[["left", "bottom"]].set_color("#666666")
        ax.tick_params(colors="#333333", direction="out", length=3)

    plt.show()
def main_k_vs_h():
    h_values = np.array([50, 100, 150, 200, 250, 300, 350, 400])
    k_values = np.array([15_000, 34_000, 58_000, 80_000, 102_000, 125_000,
                         147_000, 168_000])
    k_values_thousands = k_values / 1_000
    slope, intercept = np.polyfit(h_values, k_values, 1)
    fitted_heights = np.linspace(h_values.min(), h_values.max(), 100)
    fitted_k_values_thousands = (slope * fitted_heights + intercept) / 1_000

    fig, ax = plt.subplots(
        figsize=(10, 3.6), dpi=150, constrained_layout=True
    )
    ax.scatter(
        h_values,
        k_values_thousands,
        color="#0072B2",
        s=49,
        edgecolors="white",
        linewidths=0.8,
        label="Measured values",
        zorder=2,
    )
    ax.plot(
        fitted_heights,
        fitted_k_values_thousands,
        color="#444444",
        linestyle="--",
        linewidth=1.4,
        label=rf"Fit: $\Delta k/\Delta(H/h) \approx {slope:.0f}$",
        zorder=3,
    )

    ax.set(
        xlabel=r"Water-column height, $H/h$",
        ylabel=r"Stiffness, $k$ ($\times 10^3$)",
    )
    ax.margins(x=0.05, y=0.08)
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(loc="upper left", frameon=False, fontsize=8)

    plt.show()
def main_min_dt_at_k():
    k_values = np.array([2.5, 5, 10, 20, 40, 80, 160, 320, 640, 1280])
    dt_values = np.array(
        [0.0255, 0.019, 0.014, 0.0105, 0.0075,
         0.005, 0.003, 0.0025, 0.0015, 0.001]
    )

    exponent, log_intercept = np.polyfit(
        np.log2(k_values), np.log2(dt_values), 1
    )
    fitted_k_values = np.geomspace(k_values.min(), k_values.max(), 200)
    fitted_dt_values = 2 ** (
        exponent * np.log2(fitted_k_values) + log_intercept
    )

    fig, ax = plt.subplots(figsize=(5, 3.2), dpi=150, constrained_layout=True)
    ax.scatter(
        k_values,
        dt_values,
        color="#0072B2",
        s=49,
        edgecolors="white",
        linewidths=0.8,
        label="Experimentally det. limit",
        zorder=2,
    )
    ax.plot(
        fitted_k_values,
        fitted_dt_values,
        color="#444444",
        linestyle="--",
        linewidth=1.4,
        label=rf"Power-law fit: $dt \propto k^{{{exponent:.2f}}}$",
        zorder=1,
    )

    ax.set_xscale("log", base=2)
    ax.set_yscale("log", base=2)
    ax.set_xticks(k_values)
    ax.set_xticklabels([f"{k:g}" for k in k_values])
    dt_ticks = (0.001, 0.002, 0.004, 0.008, 0.016, 0.032)
    ax.set_yticks(dt_ticks)
    ax.set_yticklabels([f"{dt:g}" for dt in dt_ticks])
    ax.minorticks_off()

    ax.set(
        xlabel=r"Stiffness, $k$ ($\times 10^3$)",
        ylabel=r"Maximum stable timestep, $dt$ (s)",
    )
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(loc="upper right", frameon=False, fontsize=8)

    plt.show()
def main_time_by_particles():
    frame_time_ms = np.array(
        [0.7, 4.9, 9.1, 13, 16.9, 21.5, 26, 31,
         35.5, 39.5, 44, 47.7, 52.2, 56.3, 61]
    )
    rendering_time_ms = np.array(
        [0.7, 1.7, 2.3, 3.5, 3.6, 4.2, 4.9, 5.6,
         6.3, 6.8, 7.4, 8.3, 9, 9.6, 10.3]
    )
    particle_counts = np.array(
        [900, 5900, 10900, 15900, 20900, 25900, 30900, 35900,
         40900, 45900, 50900, 55900, 60900, 65900, 70900]
    )

    simulation_time_ms = frame_time_ms - rendering_time_ms
    particle_counts_thousands = particle_counts / 1_000

    fig, ax = plt.subplots(figsize=(10, 3.6), dpi=150, constrained_layout=True)
    ax.fill_between(
        particle_counts_thousands,
        0,
        simulation_time_ms,
        color="#0072B2",
        alpha=0.18,
        label="Simulation",
    )
    ax.fill_between(
        particle_counts_thousands,
        simulation_time_ms,
        frame_time_ms,
        color="#E69F00",
        alpha=0.28,
        label="Rendering and UI",
    )
    ax.plot(
        particle_counts_thousands,
        simulation_time_ms,
        color="#0072B2",
        linewidth=1.5,
        marker="o",
        markersize=4,
    )
    ax.plot(
        particle_counts_thousands,
        frame_time_ms,
        color="#333333",
        linewidth=1.6,
        marker="o",
        markersize=4,
        label="Total frame time",
    )

    ax.set(
        xlabel=r"Number of particles, n ($\times 10^3$)",
        ylabel="Time per frame (ms)",
        xlim=(0.0, None),
        ylim=(0.0, None),
    )
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(loc="upper left", frameon=False, fontsize=8)

    print(simulation_time_ms / frame_time_ms)

    plt.show()
def main_dirac_delta():
    x = np.linspace(-2.5, 2.5, 1_000)
    support_radii = (2.0, 1.0, 0.5)
    colors = ("#E69F00", "#0072B2", "#009E73")
    line_styles = ("--", "-.", "-")

    fig, ax = plt.subplots(figsize=(10, 3.6), dpi=150, constrained_layout=True)

    for support_radius, color, line_style in zip(
        support_radii, colors, line_styles):
        h = support_radius / 2.0
        q = np.abs(x) / h
        kernel = (
            np.maximum(2.0 - q, 0.0) ** 3
            - 4.0 * np.maximum(1.0 - q, 0.0) ** 3
        ) / (6.0 * h)
        kernel = np.where(np.abs(x) <= support_radius, kernel, np.nan)
        ax.plot(
            x,
            kernel,
            color=color,
            linestyle=line_style,
            linewidth=1.8,
            label=rf"Kernel, $r_s = {support_radius:g}$",
            zorder=2,
        )

    delta_height = 3.1
    ax.vlines(
        0.0,
        0.0,
        delta_height,
        color="#ff1100",
        linewidth=1.8,
        label=r"Dirac delta, $\delta(x)$",
        zorder=3,
    )
    ax.annotate(
        "",
        xy=(0.0, delta_height),
        xytext=(0.0, delta_height - 0.18),
        arrowprops={"arrowstyle": "-|>", "color": "#ff1100", "lw": 1.8},
    )
    ax.text(0.08, delta_height - 0.02, r"$\delta(x)$", color="#333333")

    ax.set(
        xlabel=r"Distance from origin, $x$",
        ylabel=r"Kernel value, $W(x, r_s)$",
        xlim=(-2.5, 2.5),
        ylim=(0.0, 3.3),
    )
    ax.set_xticks((-2, -1, 0, 1, 2))
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(loc="upper right", frameon=False, fontsize=8)

    fig.savefig(
        "Figures/dirac_delta.png",
        dpi=300,
        bbox_inches="tight",
    )

    plt.show()
def main_kernel_and_derivative():
    h = 1.0
    support_extent = 2.0 * h
    x = np.linspace(-2.5, 2.5, 1_000)
    q = np.abs(x) / h

    kernel = (np.maximum(2.0 - q, 0.0) ** 3 - 4.0 * np.maximum(1.0 - q, 0.0) ** 3) / (6.0 * h)
    kernel_derivative = np.sign(x) * (-3.0 * np.maximum(2.0 - q, 0.0) ** 2 + 12.0 * np.maximum(1.0 - q, 0.0) ** 2) / (6.0 * h**2)
    inside_support = np.abs(x) <= support_extent
    kernel = np.where(inside_support, kernel, np.nan)
    kernel_derivative = np.where(inside_support, kernel_derivative, np.nan)

    fig, ax = plt.subplots(figsize=(5, 3.6), dpi=150, constrained_layout=True)
    ax.plot(
        x,
        kernel,
        color="#0072B2",
        linewidth=1.8,
        label=rf"Kernel, $W$",
        zorder=3,
    )
    ax.plot(
        x,
        kernel_derivative,
        color="#E69F00",
        linestyle="--",
        linewidth=1.8,
        label=rf"Derivative, $\nabla W$",
        zorder=3,
    )

    ax.axhline(0.0, color="#666666", linewidth=0.8, zorder=2)
    ax.axvline(-support_extent, color="#BBBBBB", linestyle=":", linewidth=1.0)
    ax.axvline(support_extent, color="#BBBBBB", linestyle=":", linewidth=1.0)

    ax.set(
        xlabel=r"$x_j - x_i$",
        ylabel="Function value",
        xlim=(-2.5, 2.5),
        ylim=(-0.75, 0.75),
    )
    ax.set_xticks((-2, -1, 0, 1, 2))
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7)
    ax.grid(axis="x", color="#ECECEC", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.spines[["left", "bottom"]].set_color("#666666")
    ax.tick_params(colors="#333333", direction="out", length=3)
    ax.legend(loc="upper right", frameon=False, fontsize=8)

    fig.savefig(
        "Figures/kernel_and_derivative.png",
        dpi=300,
        bbox_inches="tight",
    )

    plt.show()

if __name__ == "__main__":
    try:
        main_dirac_delta()
    except (OSError, ValueError) as error:
        sys.exit(f"Error: {error}")
