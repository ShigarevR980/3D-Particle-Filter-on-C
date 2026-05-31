#include "particle_filter.h"
#include "random_utils.h"
#include "csv_writer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    // Количество частиц в фильтре.
    // Чем больше частиц, тем точнее можно приблизить распределение,
    // но тем больше вычислений требуется программе.
    const int particle_count = 1000;

    // Количество шагов симуляции.
    // На каждом шаге объект сдвигается, датчик даёт измерение,
    // а фильтр обновляет оценку положения.
    const int steps = 50;

    // dt - шаг времени между двумя соседними состояниями объекта.
    const double dt = 1.0;

    // Q - уровень процессного шума.
    // Он отвечает за случайные отклонения в движении объекта и частиц.
    const double Q = 0.05;

    // R - уровень шума измерений.
    // Чем больше R, тем менее точным считается датчик.
    const double R = 2.0;

    // alpha - коэффициент экспоненциального сглаживания скорости.
    // При alpha = 0.8 новая скорость в основном зависит от старой скорости.
    const double alpha = 0.8;

    // Фиксированный seed нужен, чтобы результат можно было повторить.
    // При одинаковом seed программа каждый раз создаёт одинаковую случайную последовательность.
    random_seed(42u);

    // Выделяем память под массив частиц.
    // Частицы хранят возможные положения, скорости и веса.
    Particle* particles = (Particle*)malloc((size_t)particle_count * sizeof(Particle));

    // Проверяем, что память действительно выделилась.
    // Если malloc вернул 0, продолжать работу нельзя.
    if (particles == 0) {
        printf("Memory allocation error\n");
        return 1;
    }

    // Создаём начальное облако частиц вокруг стартового положения объекта.
    init_particles(particles, particle_count);

    // Открываем CSV-файл для записи результатов симуляции.
    // Файл создаётся заново при каждом запуске программы.
    FILE* output = fopen("output.csv", "w");

    // Если файл открыть не удалось, освобождаем память и завершаем программу.
    if (output == 0) {
        printf("Cannot open output.csv\n");
        free(particles);
        return 1;
    }

    // Записываем первую строку CSV-файла с названиями колонок.
    // Это нужно, чтобы файл было удобно читать в Excel или Python.
    if (!write_csv_header(output)) {
        printf("Cannot write CSV header\n");
        fclose(output);
        free(particles);
        return 1;
    }

    // Истинное начальное положение объекта.
    // Фильтр напрямую это положение не знает, оно нужно для симуляции и проверки качества.
    double true_x = 0.0;
    double true_y = 0.0;
    double true_z = 0.0;

    // Истинная начальная скорость объекта.
    // Она задаёт направление движения в 3D-пространстве.
    double true_vx = 1.0;
    double true_vy = 0.5;
    double true_vz = 0.2;

    // Стандартное отклонение процессного шума для координат.
    // В коде Q задаётся как дисперсия, поэтому берём sqrt(Q).
    double process_position_stddev = sqrt(Q);

    // Шум скорости делаем меньше, чем шум координат.
    // Это помогает получить более плавное движение объекта.
    double process_velocity_stddev = sqrt(Q) * 0.3;

    // Стандартное отклонение шума измерений.
    // R тоже используется как дисперсия, поэтому берём sqrt(R).
    double measurement_stddev = sqrt(R);

    printf("3D Particle Filter simulation\n");
    printf("Particles: %d, steps: %d\n\n", particle_count, steps);

    for (int step = 0; step < steps; step++) {
        // На каждом шаге сначала моделируем истинное движение объекта.
        // Истинная скорость немного случайно меняется, чтобы движение не было идеально прямолинейным.
        double model_true_vx = true_vx + random_normal(0.0, process_velocity_stddev);
        double model_true_vy = true_vy + random_normal(0.0, process_velocity_stddev);
        double model_true_vz = true_vz + random_normal(0.0, process_velocity_stddev);

        // Сглаживаем скорость истинного объекта.
        // Это делает модель движения похожей на ту, которую использует фильтр в predict.
        true_vx = alpha * true_vx + (1.0 - alpha) * model_true_vx;
        true_vy = alpha * true_vy + (1.0 - alpha) * model_true_vy;
        true_vz = alpha * true_vz + (1.0 - alpha) * model_true_vz;

        // Обновляем истинное положение объекта.
        // К движению по скорости добавляется процессный шум.
        true_x += true_vx * dt + random_normal(0.0, process_position_stddev);
        true_y += true_vy * dt + random_normal(0.0, process_position_stddev);
        true_z += true_vz * dt + random_normal(0.0, process_position_stddev);

        // Моделируем работу датчика.
        // Датчик видит не точное истинное положение, а координаты с шумом.
        double meas_x = true_x + random_normal(0.0, measurement_stddev);
        double meas_y = true_y + random_normal(0.0, measurement_stddev);
        double meas_z = true_z + random_normal(0.0, measurement_stddev);

        // Основной цикл Particle Filter.
        // 1. predict - частицы предсказывают своё новое положение.
        // 2. update_weights - частицы сравниваются с измерением.
        // 3. normalize_weights - веса приводятся к сумме 1.
        // 4. resample - более вероятные частицы чаще попадают в новый набор.
        predict(particles, particle_count, dt, Q, alpha);
        update_weights(particles, particle_count, meas_x, meas_y, meas_z, R);
        normalize_weights(particles, particle_count);
        resample(particles, particle_count);

        // Переменные для итоговой оценки положения объекта.
        double est_x = 0.0;
        double est_y = 0.0;
        double est_z = 0.0;

        // Считаем оценку положения по текущему облаку частиц.
        estimate_position(particles, particle_count, &est_x, &est_y, &est_z);

        // Переменные для границ 95% доверительного интервала.
        // Интервал считается отдельно для каждой координаты.
        double x_low = 0.0;
        double x_high = 0.0;
        double y_low = 0.0;
        double y_high = 0.0;
        double z_low = 0.0;
        double z_high = 0.0;

        // Считаем 95% доверительный интервал по распределению частиц.
        // Для каждой координаты берутся 2.5% и 97.5% квантили.
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

        // Записываем одну строку результата в output.csv.
        // В строке хранятся истинное положение, измерение, оценка и доверительные интервалы.
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

        // Выводим краткую информацию по шагу в консоль.
        // Это помогает быстро увидеть, как оценка фильтра следует за истинным положением.
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

    // Закрываем CSV-файл, чтобы все данные точно были записаны на диск.
    fclose(output);

    // Освобождаем память, выделенную под массив частиц.
    free(particles);

    printf("\nResult saved to output.csv\n");

    return 0;
}