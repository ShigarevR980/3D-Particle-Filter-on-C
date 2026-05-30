#include "csv_writer.h"

int write_csv_header(FILE* file)
{
    if (file == 0) {
        return 0;
    }

    // Названия колонок в output.csv
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
    if (file == 0) {
        return 0;
    }

    // Одна строка — один шаг симуляции
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
        true_x, true_y, true_z,
        meas_x, meas_y, meas_z,
        est_x, est_y, est_z,
        x_low, x_high,
        y_low, y_high,
        z_low, z_high
    );

    return result >= 0;
}