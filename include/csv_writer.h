#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <stdio.h>

// Записывает первую строку с названиями колонок
int write_csv_header(FILE* file);

// Записывает одну строку результата симуляции
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
);

#endif