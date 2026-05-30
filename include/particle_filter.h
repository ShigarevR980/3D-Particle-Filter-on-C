#ifndef PARTICLE_FILTER_H
#define PARTICLE_FILTER_H

// Одна частица хранит возможное положение, скорость и вес
typedef struct {
    double x, y, z;
    double vx, vy, vz;
    double weight;
} Particle;

// Создаёт начальное облако частиц
void init_particles(Particle* particles, int N);

// Сдвигает частицы на один шаг вперёд
// Скорость обновляется через экспоненциальное сглаживание
void predict(Particle* particles, int N, double dt, double Q, double alpha);

// Обновляет веса частиц по измеренным координатам
void update_weights(
    Particle* particles,
    int N,
    double zx,
    double zy,
    double zz,
    double R
);

// Делает сумму всех весов равной 1
void normalize_weights(Particle* particles, int N);

// Оставляет более вероятные частицы и убирает слабые
void resample(Particle* particles, int N);

// Считает итоговую оценку положения объекта
void estimate_position(
    Particle* particles,
    int N,
    double* ex,
    double* ey,
    double* ez
);

// Считает 95% доверительный интервал отдельно для x, y и z
void compute_confidence_interval_95(
    Particle* particles,
    int N,
    double* x_low,
    double* x_high,
    double* y_low,
    double* y_high,
    double* z_low,
    double* z_high
);

#endif