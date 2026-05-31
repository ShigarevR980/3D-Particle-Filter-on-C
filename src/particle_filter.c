#include "particle_filter.h"
#include "random_utils.h"

#include <math.h>
#include <stdlib.h>

// Вспомогательная функция для qsort.
// Она нужна, чтобы отсортировать массив double при расчёте квантилей.
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
    // Защита от некорректного массива и неправильного количества частиц
    if (particles == 0 || N <= 0) {
        return;
    }

    // Начальное предположение о положении объекта.
    // Мы считаем, что объект стартует около точки (0, 0, 0).
    const double initial_x = 0.0;
    const double initial_y = 0.0;
    const double initial_z = 0.0;

    // Начальное предположение о скорости объекта.
    // Эти значения совпадают с базовой моделью движения в main.c.
    const double initial_vx = 1.0;
    const double initial_vy = 0.5;
    const double initial_vz = 0.2;

    // Частицы не должны быть все в одной точке.
    // Поэтому мы создаём начальное облако вокруг стартового положения.
    const double position_stddev = 2.0;
    const double velocity_stddev = 0.5;

    for (int i = 0; i < N; i++) {
        // Каждая частица получает немного отличающееся начальное положение.
        // Так фильтр сразу хранит не одну версию состояния, а облако гипотез.
        particles[i].x = initial_x + random_normal(0.0, position_stddev);
        particles[i].y = initial_y + random_normal(0.0, position_stddev);
        particles[i].z = initial_z + random_normal(0.0, position_stddev);

        // Скорость тоже задаётся с разбросом.
        // Это нужно, потому что мы не считаем начальную скорость идеально известной.
        particles[i].vx = initial_vx + random_normal(0.0, velocity_stddev);
        particles[i].vy = initial_vy + random_normal(0.0, velocity_stddev);
        particles[i].vz = initial_vz + random_normal(0.0, velocity_stddev);

        // В начале все частицы считаются одинаково вероятными.
        particles[i].weight = 1.0 / N;
    }
}

void predict(Particle* particles, int N, double dt, double Q, double alpha)
{
    // Если массив частиц некорректный, предсказывать нечего
    if (particles == 0 || N <= 0) {
        return;
    }

    // Нулевой или отрицательный шаг времени не имеет смысла для движения
    if (dt <= 0.0) {
        return;
    }

    // Шум не может быть отрицательным.
    // Если передали отрицательное значение, считаем шум нулевым.
    if (Q < 0.0) {
        Q = 0.0;
    }

    // alpha должен лежать в диапазоне [0, 1].
    // Значение 0 означает полное доверие новой модельной скорости.
    // Значение 1 означает полное сохранение старой скорости.
    if (alpha < 0.0) {
        alpha = 0.0;
    }

    if (alpha > 1.0) {
        alpha = 1.0;
    }

    // Q задаёт уровень процессного шума.
    // Для нормального распределения нужен стандартный разброс, поэтому берём sqrt(Q).
    double position_stddev = sqrt(Q);

    // Шум скорости делаем меньше, чем шум позиции.
    // Так скорость меняется плавнее и не скачет слишком резко.
    double velocity_stddev = sqrt(Q) * 0.3;

    for (int i = 0; i < N; i++) {
        // Модельная скорость - это старая скорость частицы с небольшим случайным изменением.
        // Так частицы могут учитывать, что движение объекта не идеально постоянное.
        double model_vx = particles[i].vx + random_normal(0.0, velocity_stddev);
        double model_vy = particles[i].vy + random_normal(0.0, velocity_stddev);
        double model_vz = particles[i].vz + random_normal(0.0, velocity_stddev);

        // Сглаживаем скорость через экспоненциальное среднее.
        // Это выполняет требование, что prediction не должен быть простым position += velocity.
        // Чем больше alpha, тем сильнее сохраняется старая скорость.
        particles[i].vx = alpha * particles[i].vx + (1.0 - alpha) * model_vx;
        particles[i].vy = alpha * particles[i].vy + (1.0 - alpha) * model_vy;
        particles[i].vz = alpha * particles[i].vz + (1.0 - alpha) * model_vz;

        // После обновления скорости двигаем частицу в 3D-пространстве.
        // Дополнительный шум позиции отражает неточность модели движения.
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
    // Проверяем, что массив частиц существует и количество частиц корректное
    if (particles == 0 || N <= 0) {
        return;
    }

    // R описывает шум измерения.
    // Он стоит в знаменателе, поэтому не должен быть нулём или отрицательным.
    if (R <= 0.0) {
        R = 1e-9;
    }

    for (int i = 0; i < N; i++) {
        // Сравниваем положение частицы с текущим измерением датчика.
        // zx, zy, zz - это зашумлённые координаты объекта.
        double dx = zx - particles[i].x;
        double dy = zy - particles[i].y;
        double dz = zz - particles[i].z;

        // Используем квадрат расстояния в 3D.
        // Корень не нужен, потому что в формуле правдоподобия используется именно квадрат.
        double distance_squared = dx * dx + dy * dy + dz * dz;

        // Чем ближе частица к измерению, тем больше значение likelihood.
        // Чем дальше частица, тем сильнее экспонента уменьшает её вес.
        double likelihood = exp(-0.5 * distance_squared / R);

        // Новый вес учитывает и прошлую правдоподобность частицы, и новое измерение.
        particles[i].weight *= likelihood;

        // Защита от ситуации, когда из-за очень маленьких чисел вес стал ровно нулём.
        // Это помогает избежать полного вырождения весов при нормализации.
        particles[i].weight += 1e-300;
    }
}

void normalize_weights(Particle* particles, int N)
{
    // Без массива частиц нормализовать нечего
    if (particles == 0 || N <= 0) {
        return;
    }

    double sum = 0.0;

    // Сначала считаем сумму всех весов
    for (int i = 0; i < N; i++) {
        sum += particles[i].weight;
    }

    // Если сумма оказалась нулевой или некорректной, возвращаем равные веса.
    // Это запасной вариант, чтобы фильтр мог продолжить работу.
    if (sum <= 0.0) {
        for (int i = 0; i < N; i++) {
            particles[i].weight = 1.0 / N;
        }

        return;
    }

    // Делим каждый вес на общую сумму.
    // После этого сумма всех весов становится равной 1.
    for (int i = 0; i < N; i++) {
        particles[i].weight /= sum;
    }
}

void resample(Particle* particles, int N)
{
    // Проверяем входные данные перед выделением памяти
    if (particles == 0 || N <= 0) {
        return;
    }

    // Создаём новый массив частиц.
    // В него попадут частицы, выбранные с учётом их весов.
    Particle* new_particles = (Particle*)malloc((size_t)N * sizeof(Particle));

    if (new_particles == 0) {
        return;
    }

    // Systematic resampling:
    // берём N равномерно расположенных точек на отрезке [0, 1].
    double step = 1.0 / N;

    // Случайный старт нужен, чтобы resampling не был полностью детерминированным.
    double start = random_uniform(0.0, step);

    // cumulative_weight хранит накопленную сумму весов.
    // По ней мы определяем, какая частица соответствует текущей позиции.
    double cumulative_weight = particles[0].weight;

    int j = 0;

    for (int i = 0; i < N; i++) {
        double position = start + i * step;

        // Двигаемся по накопленной сумме весов, пока не найдём частицу,
        // чей участок вероятности содержит текущую position.
        while (position > cumulative_weight && j < N - 1) {
            j++;
            cumulative_weight += particles[j].weight;
        }

        // Частица с большим весом занимает больший участок на [0, 1],
        // поэтому она может быть выбрана несколько раз.
        new_particles[i] = particles[j];

        // После resampling все частицы снова считаются равновероятными.
        new_particles[i].weight = 1.0 / N;
    }

    // Копируем новый набор частиц обратно в исходный массив
    for (int i = 0; i < N; i++) {
        particles[i] = new_particles[i];
    }

    // Освобождаем временный массив, чтобы не было утечки памяти
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
    // Проверяем не только массив частиц, но и указатели для записи результата
    if (particles == 0 || N <= 0 || ex == 0 || ey == 0 || ez == 0) {
        return;
    }

    // Начинаем сумму с нуля
    *ex = 0.0;
    *ey = 0.0;
    *ez = 0.0;

    for (int i = 0; i < N; i++) {
        // Итоговая оценка - это среднее положение частиц с учётом весов.
        // Частицы с большим весом сильнее влияют на результат.
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
    // Проверяем входные данные и все указатели для записи результата
    if (particles == 0 || N <= 0 ||
        x_low == 0 || x_high == 0 ||
        y_low == 0 || y_high == 0 ||
        z_low == 0 || z_high == 0) {
        return;
    }

    // Для расчёта квантилей нужны отдельные массивы координат.
    // Мы не сортируем сами частицы, чтобы не менять порядок основного массива.
    double* xs = (double*)malloc((size_t)N * sizeof(double));
    double* ys = (double*)malloc((size_t)N * sizeof(double));
    double* zs = (double*)malloc((size_t)N * sizeof(double));

    if (xs == 0 || ys == 0 || zs == 0) {
        free(xs);
        free(ys);
        free(zs);
        return;
    }

    // Копируем координаты частиц в отдельные массивы.
    // Для каждой координаты доверительный интервал считается отдельно.
    for (int i = 0; i < N; i++) {
        xs[i] = particles[i].x;
        ys[i] = particles[i].y;
        zs[i] = particles[i].z;
    }

    // Сортируем координаты, чтобы взять 2.5% и 97.5% квантили.
    qsort(xs, (size_t)N, sizeof(double), compare_double);
    qsort(ys, (size_t)N, sizeof(double), compare_double);
    qsort(zs, (size_t)N, sizeof(double), compare_double);

    // Для 95% интервала отбрасываем примерно 2.5% частиц снизу
    // и 2.5% частиц сверху по каждой координате.
    int low_index = (int)(0.025 * (N - 1));
    int high_index = (int)(0.975 * (N - 1));

    // Границы интервала по координате X
    *x_low = xs[low_index];
    *x_high = xs[high_index];

    // Границы интервала по координате Y
    *y_low = ys[low_index];
    *y_high = ys[high_index];

    // Границы интервала по координате Z
    *z_low = zs[low_index];
    *z_high = zs[high_index];

    // Освобождаем временные массивы
    free(xs);
    free(ys);
    free(zs);
}