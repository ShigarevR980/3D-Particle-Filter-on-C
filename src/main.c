#include "particle_filter.h"
#include "random_utils.h"
#include "csv_writer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const int particle_count = 1000;
    const int steps = 50;

    const double dt = 1.0;
    const double Q = 0.05;
    const double R = 2.0;
    const double alpha = 0.8;

    // Фиксированный seed нужен, чтобы результат можно было повторить
    random_seed(42u);

    Particle* particles = (Particle*)malloc((size_t)particle_count * sizeof(Particle));

    if (particles == 0) {
        printf("Memory allocation error\n");
        return 1;
    }

    init_particles(particles, particle_count);

    FILE* output = fopen("output.csv", "w");

    if (output == 0) {
        printf("Cannot open output.csv\n");
        free(particles);
        return 1;
    }

    if (!write_csv_header(output)) {
        printf("Cannot write CSV header\n");
        fclose(output);
        free(particles);
        return 1;
    }

    // Истинное начальное состояние объекта
    double true_x = 0.0;
    double true_y = 0.0;
    double true_z = 0.0;

    double true_vx = 1.0;
    double true_vy = 0.5;
    double true_vz = 0.2;

    double process_position_stddev = sqrt(Q);
    double process_velocity_stddev = sqrt(Q) * 0.3;
    double measurement_stddev = sqrt(R);

    printf("3D Particle Filter simulation\n");
    printf("Particles: %d, steps: %d\n\n", particle_count, steps);

    for (int step = 0; step < steps; step++) {
        // Немного меняем истинную скорость объекта
        double model_true_vx = true_vx + random_normal(0.0, process_velocity_stddev);
        double model_true_vy = true_vy + random_normal(0.0, process_velocity_stddev);
        double model_true_vz = true_vz + random_normal(0.0, process_velocity_stddev);

        // Сглаживаем скорость истинного объекта для более плавного движения
        true_vx = alpha * true_vx + (1.0 - alpha) * model_true_vx;
        true_vy = alpha * true_vy + (1.0 - alpha) * model_true_vy;
        true_vz = alpha * true_vz + (1.0 - alpha) * model_true_vz;

        // Обновляем истинное положение объекта
        true_x += true_vx * dt + random_normal(0.0, process_position_stddev);
        true_y += true_vy * dt + random_normal(0.0, process_position_stddev);
        true_z += true_vz * dt + random_normal(0.0, process_position_stddev);

        // Датчик видит координаты с шумом
        double meas_x = true_x + random_normal(0.0, measurement_stddev);
        double meas_y = true_y + random_normal(0.0, measurement_stddev);
        double meas_z = true_z + random_normal(0.0, measurement_stddev);

        // Основной цикл Particle Filter
        predict(particles, particle_count, dt, Q, alpha);
        update_weights(particles, particle_count, meas_x, meas_y, meas_z, R);
        normalize_weights(particles, particle_count);
        resample(particles, particle_count);

        double est_x = 0.0;
        double est_y = 0.0;
        double est_z = 0.0;

        estimate_position(particles, particle_count, &est_x, &est_y, &est_z);

        double x_low = 0.0;
        double x_high = 0.0;
        double y_low = 0.0;
        double y_high = 0.0;
        double z_low = 0.0;
        double z_high = 0.0;

        compute_confidence_interval_95(
            particles,
            particle_count,
            &x_low,
            &x_high,
            &y_low,
            &y_high,
            &z_low,
            &z_high
        );

        if (!write_csv_row(
            output,
            step,
            true_x,
            true_y,
            true_z,
            meas_x,
            meas_y,
            meas_z,
            est_x,
            est_y,
            est_z,
            x_low,
            x_high,
            y_low,
            y_high,
            z_low,
            z_high
        )) {
            printf("Cannot write CSV row at step %d\n", step);
            fclose(output);
            free(particles);
            return 1;
        }

        printf(
            "step %02d | true=(%.2f, %.2f, %.2f) | meas=(%.2f, %.2f, %.2f) | est=(%.2f, %.2f, %.2f)\n",
            step,
            true_x,
            true_y,
            true_z,
            meas_x,
            meas_y,
            meas_z,
            est_x,
            est_y,
            est_z
        );
    }

    fclose(output);
    free(particles);

    printf("\nResult saved to output.csv\n");

    return 0;
}