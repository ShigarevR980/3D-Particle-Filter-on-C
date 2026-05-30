#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

// Задаёт стартовое значение для генератора случайных чисел
void random_seed(unsigned int seed);

// Возвращает случайное число от min до max
double random_uniform(double min, double max);

// Возвращает случайное число из нормального распределения
double random_normal(double mean, double stddev);

#endif