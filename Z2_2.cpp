// Задание 2: Вычисление sin(x) через ряд Тейлора с заданной точностью
#include <iostream>
#include <cmath>
#include <iomanip>

int main() {
    double x, eps;
    std::cout << "Введите x (в радианах) и точность (например, 0.00001): ";
    if (!(std::cin >> x >> eps)) {
        std::cerr << "Ошибка ввода\n";
        return 1;
    }

    // Для лучшей сходимости можно привести x к диапазону [-π, π]
    x = std::fmod(x, 2 * M_PI);
    if (x > M_PI)        x -= 2 * M_PI;
    else if (x < -M_PI)  x += 2 * M_PI;

    double term = x;          // первый член ряда: x
    double sum  = term;       // накопленная сумма
    double x2   = x * x;      // x^2 для ускорения вычислений
    int n = 1;                // индекс очередного члена

    // Генерируем следующий член через предыдущий: 
    // term_n = term_{n-1} * ( - x^2 / [(2n)*(2n+1)] )
    while (std::fabs(term) >= eps) {
        term *= - x2 / ((2 * n) * (2 * n + 1));
        sum += term;
        ++n;
    }

    double lib_sin = std::sin(x);

    std::cout << std::fixed << std::setprecision(10)
              << "sin(x) по ряду Тейлора = " << sum << "\n"
              << "std::sin(x)           = " << lib_sin << "\n"
              << "Разница               = " << (sum - lib_sin) << "\n";

    return 0;
}
