#include "particle_filter.h"
#include "random_utils.h"

#include <math.h>
#include <stdlib.h>

static int compare_double(const void* a, const void* b)
{
    double da = *(const double*)a;
    double db = *(const double*)b;

    if (da < db) {
        return -1;
    }

    if (da > db) {
        return 1;
    }

    return 0;
}

void init_particles(Particle* particles, int N)
{
    if (particles == 0 || N <= 0) {
        return;
    }

    // Начальное предположение о положении объекта
    const double initial_x = 0.0;
    const double initial_y = 0.0;
    const double initial_z = 0.0;

    // Начальное предположение о скорости объекта
    const double initial_vx = 1.0;
    const double initial_vy = 0.5;
    const double initial_vz = 0.2;

    // Насколько широко разбросать частицы в начале
    const double position_stddev = 2.0;
    const double velocity_stddev = 0.5;

    for (int i = 0; i < N; i++) {
        particles[i].x = initial_x + random_normal(0.0, position_stddev);
        particles[i].y = initial_y + random_normal(0.0, position_stddev);
        particles[i].z = initial_z + random_normal(0.0, position_stddev);

        particles[i].vx = initial_vx + random_normal(0.0, velocity_stddev);
        particles[i].vy = initial_vy + random_normal(0.0, velocity_stddev);
        particles[i].vz = initial_vz + random_normal(0.0, velocity_stddev);

        particles[i].weight = 1.0 / N;
    }
}

void predict(Particle* particles, int N, double dt, double Q, double alpha)
{
    if (particles == 0 || N <= 0) {
        return;
    }

    if (dt <= 0.0) {
        return;
    }

    if (Q < 0.0) {
        Q = 0.0;
    }

    if (alpha < 0.0) {
        alpha = 0.0;
    }

    if (alpha > 1.0) {
        alpha = 1.0;
    }

    double position_stddev = sqrt(Q);
    double velocity_stddev = sqrt(Q) * 0.3;

    for (int i = 0; i < N; i++) {
        // Модельная скорость немного отличается от старой скорости
        double model_vx = particles[i].vx + random_normal(0.0, velocity_stddev);
        double model_vy = particles[i].vy + random_normal(0.0, velocity_stddev);
        double model_vz = particles[i].vz + random_normal(0.0, velocity_stddev);

        // Сглаживаем скорость, чтобы она не менялась резко
        // Новая скорость = большая часть старой скорости + маленькая часть новой скорости
        particles[i].vx = alpha * particles[i].vx + (1.0 - alpha) * model_vx;
        particles[i].vy = alpha * particles[i].vy + (1.0 - alpha) * model_vy;
        particles[i].vz = alpha * particles[i].vz + (1.0 - alpha) * model_vz;

        // Двигаем частицу по сглаженной скорости и добавляем шум
        particles[i].x += particles[i].vx * dt + random_normal(0.0, position_stddev);
        particles[i].y += particles[i].vy * dt + random_normal(0.0, position_stddev);
        particles[i].z += particles[i].vz * dt + random_normal(0.0, position_stddev);
    }
}

void update_weights(
    Particle* particles,
    int N,
    double zx,
    double zy,
    double zz,
    double R
)
{
    if (particles == 0 || N <= 0) {
        return;
    }

    if (R <= 0.0) {
        R = 1e-9;
    }

    for (int i = 0; i < N; i++) {
        // Считаем расстояние между частицей и измерением
        double dx = zx - particles[i].x;
        double dy = zy - particles[i].y;
        double dz = zz - particles[i].z;

        double distance_squared = dx * dx + dy * dy + dz * dz;

        // Чем дальше частица от измерения, тем меньше её вес
        double likelihood = exp(-0.5 * distance_squared / R);

        // Учитываем старый вес частицы
        particles[i].weight *= likelihood;

        // Маленькая защита от полного обнуления веса
        particles[i].weight += 1e-300;
    }
}

void normalize_weights(Particle* particles, int N)
{
    if (particles == 0 || N <= 0) {
        return;
    }

    double sum = 0.0;

    for (int i = 0; i < N; i++) {
        sum += particles[i].weight;
    }

    // Если все веса стали нулевыми, возвращаем равные веса
    if (sum <= 0.0) {
        for (int i = 0; i < N; i++) {
            particles[i].weight = 1.0 / N;
        }

        return;
    }

    for (int i = 0; i < N; i++) {
        particles[i].weight /= sum;
    }
}

void resample(Particle* particles, int N)
{
    if (particles == 0 || N <= 0) {
        return;
    }

    Particle* new_particles = (Particle*)malloc((size_t)N * sizeof(Particle));

    if (new_particles == 0) {
        return;
    }

    double step = 1.0 / N;
    double start = random_uniform(0.0, step);
    double cumulative_weight = particles[0].weight;

    int j = 0;

    for (int i = 0; i < N; i++) {
        double position = start + i * step;

        // Идём по накопленной сумме весов, пока не найдём нужную частицу
        while (position > cumulative_weight && j < N - 1) {
            j++;
            cumulative_weight += particles[j].weight;
        }

        new_particles[i] = particles[j];

        // После resampling все частицы снова имеют одинаковый вес
        new_particles[i].weight = 1.0 / N;
    }

    for (int i = 0; i < N; i++) {
        particles[i] = new_particles[i];
    }

    free(new_particles);
}

void estimate_position(
    Particle* particles,
    int N,
    double* ex,
    double* ey,
    double* ez
)
{
    if (particles == 0 || N <= 0 || ex == 0 || ey == 0 || ez == 0) {
        return;
    }

    *ex = 0.0;
    *ey = 0.0;
    *ez = 0.0;

    for (int i = 0; i < N; i++) {
        // Итоговая оценка — среднее положение частиц с учётом весов
        *ex += particles[i].x * particles[i].weight;
        *ey += particles[i].y * particles[i].weight;
        *ez += particles[i].z * particles[i].weight;
    }
}

void compute_confidence_interval_95(
    Particle* particles,
    int N,
    double* x_low,
    double* x_high,
    double* y_low,
    double* y_high,
    double* z_low,
    double* z_high
)
{
    if (particles == 0 || N <= 0 ||
        x_low == 0 || x_high == 0 ||
        y_low == 0 || y_high == 0 ||
        z_low == 0 || z_high == 0) {
        return;
    }

    double* xs = (double*)malloc((size_t)N * sizeof(double));
    double* ys = (double*)malloc((size_t)N * sizeof(double));
    double* zs = (double*)malloc((size_t)N * sizeof(double));

    if (xs == 0 || ys == 0 || zs == 0) {
        free(xs);
        free(ys);
        free(zs);
        return;
    }

    for (int i = 0; i < N; i++) {
        xs[i] = particles[i].x;
        ys[i] = particles[i].y;
        zs[i] = particles[i].z;
    }

    // Сортируем координаты, чтобы взять нужные квантили
    qsort(xs, (size_t)N, sizeof(double), compare_double);
    qsort(ys, (size_t)N, sizeof(double), compare_double);
    qsort(zs, (size_t)N, sizeof(double), compare_double);

    int low_index = (int)(0.025 * (N - 1));
    int high_index = (int)(0.975 * (N - 1));

    *x_low = xs[low_index];
    *x_high = xs[high_index];

    *y_low = ys[low_index];
    *y_high = ys[high_index];

    *z_low = zs[low_index];
    *z_high = zs[high_index];

    free(xs);
    free(ys);
    free(zs);
}