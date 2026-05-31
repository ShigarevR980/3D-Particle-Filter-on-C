#include "particle_filter.h"
#include "random_utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    // В тесте используем меньше частиц, чем в основной программе.
    // Этого достаточно, чтобы проверить вызовы функций, и тест работает быстрее.
    const int particle_count = 100;

    // Параметры такие же по смыслу, как в main.c.
    // Здесь они нужны для короткого тестового запуска фильтра.
    const double dt = 1.0;
    const double Q = 0.05;
    const double R = 2.0;
    const double alpha = 0.8;

    // Фиксированный seed делает тест повторяемым.
    // При каждом запуске будут генерироваться одинаковые случайные значения.
    random_seed(123u);

    // Выделяем память под небольшой массив частиц.
    Particle* particles = (Particle*)malloc((size_t)particle_count * sizeof(Particle));

    // Если память не выделилась, тест сразу считается проваленным.
    if (particles == 0) {
        printf("[FAIL] memory allocation error\n");
        return 1;
    }

    // Создаём начальное облако частиц.
    init_particles(particles, particle_count);

    // Выполняем один короткий цикл фильтра частиц.
    // Здесь нет полной симуляции, как в main.c.
    // Цель smoke test - проверить, что основные функции вызываются и не ломаются.
    predict(particles, particle_count, dt, Q, alpha);

    // Используем фиксированное тестовое измерение.
    // Оно имитирует координаты, которые мог бы вернуть датчик.
    update_weights(particles, particle_count, 1.0, 0.5, 0.2, R);

    // После обновления весов обязательно нормализуем их.
    normalize_weights(particles, particle_count);

    // Выполняем resampling, чтобы проверить этот этап фильтра.
    resample(particles, particle_count);

    // Переменные для оценки положения объекта.
    double est_x = 0.0;
    double est_y = 0.0;
    double est_z = 0.0;

    // Проверяем, что функция оценки положения отрабатывает корректно.
    estimate_position(particles, particle_count, &est_x, &est_y, &est_z);

    // Переменные для границ 95% доверительного интервала.
    double x_low = 0.0;
    double x_high = 0.0;
    double y_low = 0.0;
    double y_high = 0.0;
    double z_low = 0.0;
    double z_high = 0.0;

    // Проверяем, что доверительный интервал можно посчитать на текущем наборе частиц.
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

    // Проверяем, что оценка не стала NaN.
    // NaN мог бы означать ошибку в вычислениях, например неправильную нормализацию весов.
    if (isnan(est_x) || isnan(est_y) || isnan(est_z)) {
        printf("[FAIL] estimate contains NaN\n");
        free(particles);
        return 1;
    }

    // Проверяем, что границы интервала идут в правильном порядке.
    // Нижняя граница не должна быть больше верхней.
    if (x_low > x_high || y_low > y_high || z_low > z_high) {
        printf("[FAIL] confidence interval is invalid\n");
        free(particles);
        return 1;
    }

    // Освобождаем память после теста.
    free(particles);

    // Если программа дошла до этого места, базовый цикл фильтра работает без критических ошибок.
    printf("[OK] smoke test passed\n");

    return 0;
}