import csv
import math
import os

import matplotlib.pyplot as plt


OUTPUT_CSV = "output.csv"
PLOTS_DIR = "plots"

METRICS_TXT = "metrics.txt"
METRICS_CSV = "metrics.csv"


# Базовые настройки внешнего вида графиков
plt.rcParams["font.family"] = "DejaVu Sans"
plt.rcParams["font.size"] = 11
plt.rcParams["axes.titlesize"] = 16
plt.rcParams["axes.labelsize"] = 13
plt.rcParams["legend.fontsize"] = 11
plt.rcParams["xtick.labelsize"] = 11
plt.rcParams["ytick.labelsize"] = 11
plt.rcParams["figure.dpi"] = 120
plt.rcParams["savefig.dpi"] = 220


def read_csv(filename):
    rows = []

    with open(filename, "r", encoding="utf-8") as file:
        reader = csv.DictReader(file)

        for row in reader:
            rows.append({
                key: float(value) if key != "step" else int(value)
                for key, value in row.items()
            })

    return rows


def get_column(rows, name):
    return [row[name] for row in rows]


def ensure_plots_dir():
    os.makedirs(PLOTS_DIR, exist_ok=True)


def save_figure(fig, filename):
    ensure_plots_dir()

    path = os.path.join(PLOTS_DIR, filename)
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)

    print(f"Сохранён график: {path}")


def compute_metrics(rows):
    true_x = get_column(rows, "true_x")
    true_y = get_column(rows, "true_y")
    true_z = get_column(rows, "true_z")

    est_x = get_column(rows, "est_x")
    est_y = get_column(rows, "est_y")
    est_z = get_column(rows, "est_z")

    error_x = [abs(t - e) for t, e in zip(true_x, est_x)]
    error_y = [abs(t - e) for t, e in zip(true_y, est_y)]
    error_z = [abs(t - e) for t, e in zip(true_z, est_z)]

    error_3d = [
        math.sqrt((tx - ex) ** 2 + (ty - ey) ** 2 + (tz - ez) ** 2)
        for tx, ty, tz, ex, ey, ez in zip(true_x, true_y, true_z, est_x, est_y, est_z)
    ]

    def mean(values):
        return sum(values) / len(values) if values else 0.0

    def rmse(values):
        return math.sqrt(sum(v * v for v in values) / len(values)) if values else 0.0

    return {
        "mae_x": mean(error_x),
        "mae_y": mean(error_y),
        "mae_z": mean(error_z),
        "mae_3d": mean(error_3d),

        "rmse_x": rmse(error_x),
        "rmse_y": rmse(error_y),
        "rmse_z": rmse(error_z),
        "rmse_3d": rmse(error_3d),

        "max_3d": max(error_3d) if error_3d else 0.0,

        "error_x": error_x,
        "error_y": error_y,
        "error_z": error_z,
        "error_3d": error_3d,
    }


def print_metrics(metrics):
    print("\nМетрики качества отслеживания:")
    print(f"MAE по X:   {metrics['mae_x']:.4f}")
    print(f"MAE по Y:   {metrics['mae_y']:.4f}")
    print(f"MAE по Z:   {metrics['mae_z']:.4f}")
    print(f"MAE в 3D:   {metrics['mae_3d']:.4f}")
    print(f"RMSE по X:  {metrics['rmse_x']:.4f}")
    print(f"RMSE по Y:  {metrics['rmse_y']:.4f}")
    print(f"RMSE по Z:  {metrics['rmse_z']:.4f}")
    print(f"RMSE в 3D:  {metrics['rmse_3d']:.4f}")
    print(f"Max 3D err: {metrics['max_3d']:.4f}")


def save_metrics(metrics):
    ensure_plots_dir()

    txt_path = os.path.join(PLOTS_DIR, METRICS_TXT)
    csv_path = os.path.join(PLOTS_DIR, METRICS_CSV)

    with open(txt_path, "w", encoding="utf-8") as file:
        file.write("Метрики качества отслеживания\n")
        file.write("=============================\n\n")
        file.write(f"MAE по X:        {metrics['mae_x']:.6f}\n")
        file.write(f"MAE по Y:        {metrics['mae_y']:.6f}\n")
        file.write(f"MAE по Z:        {metrics['mae_z']:.6f}\n")
        file.write(f"MAE в 3D:        {metrics['mae_3d']:.6f}\n")
        file.write(f"RMSE по X:       {metrics['rmse_x']:.6f}\n")
        file.write(f"RMSE по Y:       {metrics['rmse_y']:.6f}\n")
        file.write(f"RMSE по Z:       {metrics['rmse_z']:.6f}\n")
        file.write(f"RMSE в 3D:       {metrics['rmse_3d']:.6f}\n")
        file.write(f"Макс. ошибка 3D: {metrics['max_3d']:.6f}\n")

    with open(csv_path, "w", encoding="utf-8", newline="") as file:
        writer = csv.writer(file)

        writer.writerow(["metric", "value"])
        writer.writerow(["mae_x", f"{metrics['mae_x']:.6f}"])
        writer.writerow(["mae_y", f"{metrics['mae_y']:.6f}"])
        writer.writerow(["mae_z", f"{metrics['mae_z']:.6f}"])
        writer.writerow(["mae_3d", f"{metrics['mae_3d']:.6f}"])
        writer.writerow(["rmse_x", f"{metrics['rmse_x']:.6f}"])
        writer.writerow(["rmse_y", f"{metrics['rmse_y']:.6f}"])
        writer.writerow(["rmse_z", f"{metrics['rmse_z']:.6f}"])
        writer.writerow(["rmse_3d", f"{metrics['rmse_3d']:.6f}"])
        writer.writerow(["max_3d_error", f"{metrics['max_3d']:.6f}"])

    print(f"Сохранены метрики: {txt_path}")
    print(f"Сохранены метрики: {csv_path}")


def plot_3d_trajectory(rows):
    true_x = get_column(rows, "true_x")
    true_y = get_column(rows, "true_y")
    true_z = get_column(rows, "true_z")

    meas_x = get_column(rows, "meas_x")
    meas_y = get_column(rows, "meas_y")
    meas_z = get_column(rows, "meas_z")

    est_x = get_column(rows, "est_x")
    est_y = get_column(rows, "est_y")
    est_z = get_column(rows, "est_z")

    fig = plt.figure(figsize=(11, 8))
    ax = fig.add_subplot(111, projection="3d")

    ax.plot(
        true_x, true_y, true_z,
        linewidth=2.8,
        color="#1f77b4",
        label="Истинная траектория"
    )

    ax.plot(
        est_x, est_y, est_z,
        linewidth=2.6,
        linestyle="--",
        color="#ff7f0e",
        label="Оценка фильтра частиц"
    )

    ax.scatter(
        meas_x, meas_y, meas_z,
        s=32,
        alpha=0.45,
        color="#4c9ed9",
        label="Зашумлённые измерения"
    )

    ax.scatter(
        true_x[0], true_y[0], true_z[0],
        s=120,
        color="#ff7f0e",
        marker="o",
        label="Старт"
    )

    ax.scatter(
        true_x[-1], true_y[-1], true_z[-1],
        s=150,
        color="#2ca02c",
        marker="X",
        label="Финиш"
    )

    ax.set_title("Отслеживание объекта в 3D с помощью фильтра частиц", pad=18)
    ax.set_xlabel("Координата X", labelpad=10)
    ax.set_ylabel("Координата Y", labelpad=10)
    ax.set_zlabel("Координата Z", labelpad=10)

    ax.view_init(elev=24, azim=-61)
    ax.grid(True, alpha=0.35)
    ax.legend(loc="upper right", framealpha=0.95)

    save_figure(fig, "trajectory_3d.png")


def plot_coordinates_with_confidence_intervals(rows):
    steps = get_column(rows, "step")

    coordinates = [
        ("X", "true_x", "meas_x", "est_x", "x_low", "x_high"),
        ("Y", "true_y", "meas_y", "est_y", "y_low", "y_high"),
        ("Z", "true_z", "meas_z", "est_z", "z_low", "z_high"),
    ]

    fig, axes = plt.subplots(3, 1, figsize=(13, 11), sharex=True)

    common_handles = None
    common_labels = None

    for index, (ax, (name, true_col, meas_col, est_col, low_col, high_col)) in enumerate(zip(axes, coordinates)):
        true_values = get_column(rows, true_col)
        meas_values = get_column(rows, meas_col)
        est_values = get_column(rows, est_col)
        low_values = get_column(rows, low_col)
        high_values = get_column(rows, high_col)

        band = ax.fill_between(
            steps,
            low_values,
            high_values,
            color="#7fb3d5",
            alpha=0.30,
            label="95% доверительный интервал"
        )

        true_line, = ax.plot(
            steps,
            true_values,
            linewidth=2.4,
            color="#1f77b4",
            label="Истинное значение"
        )

        est_line, = ax.plot(
            steps,
            est_values,
            linewidth=2.3,
            linestyle="--",
            color="#ff7f0e",
            label="Оценка фильтра"
        )

        meas_points = ax.scatter(
            steps,
            meas_values,
            s=24,
            alpha=0.55,
            color="#f28e2b",
            edgecolors="none",
            label="Измерение"
        )

        ax.set_ylabel(f"Координата {name}")
        ax.grid(True, alpha=0.30)

        if index == 0:
            common_handles = [band, true_line, est_line, meas_points]
            common_labels = [
                "95% доверительный интервал",
                "Истинное значение",
                "Оценка фильтра",
                "Измерение",
            ]

    axes[-1].set_xlabel("Шаг симуляции")
    fig.suptitle("Координаты объекта и 95% доверительные интервалы", y=0.98)

    fig.legend(
        common_handles,
        common_labels,
        loc="upper center",
        ncol=4,
        frameon=True,
        bbox_to_anchor=(0.5, 0.95)
    )

    fig.tight_layout(rect=[0, 0, 1, 0.93])
    save_figure(fig, "coordinates_with_confidence_intervals.png")


def plot_tracking_error(rows, metrics):
    steps = get_column(rows, "step")

    error_x = metrics["error_x"]
    error_y = metrics["error_y"]
    error_z = metrics["error_z"]
    error_3d = metrics["error_3d"]

    fig, ax = plt.subplots(figsize=(13, 6.5))

    ax.plot(steps, error_x, linewidth=2.3, color="#1f77b4", label="Ошибка по X")
    ax.plot(steps, error_y, linewidth=2.3, color="#ff7f0e", label="Ошибка по Y")
    ax.plot(steps, error_z, linewidth=2.3, color="#2ca02c", label="Ошибка по Z")

    ax.plot(
        steps,
        error_3d,
        linewidth=3.0,
        linestyle="--",
        color="#d62728",
        label="Общая ошибка в 3D"
    )

    mean_3d = metrics["mae_3d"]

    ax.axhline(
        mean_3d,
        color="#8c564b",
        linestyle=":",
        linewidth=2.0,
        label=f"Средняя 3D-ошибка = {mean_3d:.2f}"
    )

    metrics_text = (
        f"MAE X = {metrics['mae_x']:.2f}\n"
        f"MAE Y = {metrics['mae_y']:.2f}\n"
        f"MAE Z = {metrics['mae_z']:.2f}\n"
        f"MAE 3D = {metrics['mae_3d']:.2f}\n"
        f"RMSE 3D = {metrics['rmse_3d']:.2f}\n"
        f"Max 3D = {metrics['max_3d']:.2f}"
    )

    ax.text(
        0.015,
        0.97,
        metrics_text,
        transform=ax.transAxes,
        va="top",
        ha="left",
        bbox=dict(
            boxstyle="round",
            facecolor="white",
            alpha=0.9,
            edgecolor="#bbbbbb"
        )
    )

    ax.set_title("Ошибка отслеживания по шагам симуляции")
    ax.set_xlabel("Шаг симуляции")
    ax.set_ylabel("Ошибка")
    ax.grid(True, alpha=0.30)
    ax.legend(loc="upper right", framealpha=0.95)

    fig.tight_layout()
    save_figure(fig, "tracking_error.png")


def plot_2d_projections(rows):
    true_x = get_column(rows, "true_x")
    true_y = get_column(rows, "true_y")
    true_z = get_column(rows, "true_z")

    meas_x = get_column(rows, "meas_x")
    meas_y = get_column(rows, "meas_y")
    meas_z = get_column(rows, "meas_z")

    est_x = get_column(rows, "est_x")
    est_y = get_column(rows, "est_y")
    est_z = get_column(rows, "est_z")

    projections = [
        (
            "Проекция на плоскость XY",
            true_x, true_y,
            meas_x, meas_y,
            est_x, est_y,
            "Координата X",
            "Координата Y",
        ),
        (
            "Проекция на плоскость XZ",
            true_x, true_z,
            meas_x, meas_z,
            est_x, est_z,
            "Координата X",
            "Координата Z",
        ),
        (
            "Проекция на плоскость YZ",
            true_y, true_z,
            meas_y, meas_z,
            est_y, est_z,
            "Координата Y",
            "Координата Z",
        ),
    ]

    fig, axes = plt.subplots(1, 3, figsize=(18, 5.8))

    common_handles = None
    common_labels = None

    for index, (ax, (title, tx, ty, mx, my, ex, ey, xlabel, ylabel)) in enumerate(zip(axes, projections)):
        true_line, = ax.plot(
            tx,
            ty,
            linewidth=2.4,
            color="#1f77b4",
            label="Истинная траектория"
        )

        est_line, = ax.plot(
            ex,
            ey,
            linewidth=2.3,
            linestyle="--",
            color="#ff7f0e",
            label="Оценка фильтра"
        )

        meas_points = ax.scatter(
            mx,
            my,
            s=26,
            alpha=0.50,
            color="#4c9ed9",
            label="Измерения"
        )

        ax.set_title(title)
        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        ax.grid(True, alpha=0.30)

        if index == 0:
            common_handles = [true_line, est_line, meas_points]
            common_labels = [
                "Истинная траектория",
                "Оценка фильтра",
                "Измерения",
            ]

    fig.suptitle("2D-проекции трёхмерной траектории", y=0.98)

    fig.legend(
        common_handles,
        common_labels,
        loc="upper center",
        ncol=3,
        frameon=True,
        bbox_to_anchor=(0.5, 0.93)
    )

    fig.tight_layout(rect=[0, 0, 1, 0.90])
    save_figure(fig, "trajectory_projections.png")


def main():
    if not os.path.exists(OUTPUT_CSV):
        print("Файл output.csv не найден.")
        print("Сначала запусти C-программу командой: make run")
        return 1

    rows = read_csv(OUTPUT_CSV)

    if not rows:
        print("Файл output.csv пустой.")
        return 1

    metrics = compute_metrics(rows)

    print_metrics(metrics)
    save_metrics(metrics)

    plot_3d_trajectory(rows)
    plot_coordinates_with_confidence_intervals(rows)
    plot_tracking_error(rows, metrics)
    plot_2d_projections(rows)

    print("\nВсе графики и метрики сохранены в папку plots.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())