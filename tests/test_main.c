#include "particle_filter.h"
#include "random_utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const int particle_count = 100;
    const double dt = 1.0;
    const double Q = 0.05;
    const double R = 2.0;
    const double alpha = 0.8;

    random_seed(123u);

    Particle* particles = (Particle*)malloc((size_t)particle_count * sizeof(Particle));

    if (particles == 0) {
        printf("[FAIL] memory allocation error\n");
        return 1;
    }

    init_particles(particles, particle_count);

    predict(particles, particle_count, dt, Q, alpha);
    update_weights(particles, particle_count, 1.0, 0.5, 0.2, R);
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

    // Проверяем, что оценка не стала NaN
    if (isnan(est_x) || isnan(est_y) || isnan(est_z)) {
        printf("[FAIL] estimate contains NaN\n");
        free(particles);
        return 1;
    }

    // Проверяем, что границы интервала идут в правильном порядке
    if (x_low > x_high || y_low > y_high || z_low > z_high) {
        printf("[FAIL] confidence interval is invalid\n");
        free(particles);
        return 1;
    }

    free(particles);

    printf("[OK] smoke test passed\n");

    return 0;
}