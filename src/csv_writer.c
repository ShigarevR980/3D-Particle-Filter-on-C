#include "csv_writer.h"

int write_csv_header(FILE* file)
{
    // Проверяем, что файл действительно открыт.
    // Если file == 0, значит fopen не смог открыть или создать output.csv.
    if (file == 0) {
        return 0;
    }

    // Записываем первую строку CSV-файла.
    // В ней находятся названия всех колонок.
    // Эти названия потом используются Python-скриптом для построения графиков.
    int result = fprintf(
        file,
        "step,"
        "true_x,true_y,true_z,"
        "meas_x,meas_y,meas_z,"
        "est_x,est_y,est_z,"
        "x_low,x_high,"
        "y_low,y_high,"
        "z_low,z_high\n"
    );

    // fprintf возвращает отрицательное значение, если запись не удалась.
    // Возвращаем 1 при успехе и 0 при ошибке.
    return result >= 0;
}

int write_csv_row(
    FILE* file,
    int step,
    double true_x,
    double true_y,
    double true_z,
    double meas_x,
    double meas_y,
    double meas_z,
    double est_x,
    double est_y,
    double est_z,
    double x_low,
    double x_high,
    double y_low,
    double y_high,
    double z_low,
    double z_high
)
{
    // Проверяем, что файл открыт перед записью строки.
    if (file == 0) {
        return 0;
    }

    // Одна строка CSV соответствует одному шагу симуляции.
    // Порядок значений должен совпадать с порядком колонок из write_csv_header.
    int result = fprintf(
        file,
        "%d,"
        "%.6f,%.6f,%.6f,"
        "%.6f,%.6f,%.6f,"
        "%.6f,%.6f,%.6f,"
        "%.6f,%.6f,"
        "%.6f,%.6f,"
        "%.6f,%.6f\n",
        step,

        // Истинное положение объекта.
        true_x, true_y, true_z,

        // Зашумлённое измерение датчика.
        meas_x, meas_y, meas_z,

        // Оценка положения, полученная фильтром частиц.
        est_x, est_y, est_z,

        // Границы 95% доверительного интервала по X.
        x_low, x_high,

        // Границы 95% доверительного интервала по Y.
        y_low, y_high,

        // Границы 95% доверительного интервала по Z.
        z_low, z_high
    );

    // Возвращаем 1, если строка успешно записана, иначе 0.
    // Это позволяет main.c отследить ошибку записи в output.csv.
    return result >= 0;
}