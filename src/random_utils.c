#include "random_utils.h"

#include <math.h>
#include <stdlib.h>

void random_seed(unsigned int seed)
{
    srand(seed);
}

double random_uniform(double min, double max)
{
    double u = (double)rand() / (double)RAND_MAX;
    return min + (max - min) * u;
}

double random_normal(double mean, double stddev)
{
    const double pi = 3.14159265358979323846;

    // Берём два равномерных случайных числа
    // +1 и +2 нужны, чтобы не получить log(0)
    double u1 = ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0);
    double u2 = ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0);

    // Преобразование Бокса-Мюллера даёт нормальный шум
    double radius = sqrt(-2.0 * log(u1));
    double angle = 2.0 * pi * u2;

    double z = radius * cos(angle);

    return mean + stddev * z;
}