// Задание 1: Нахождение корней квадратного уравнения
#include <iostream>
#include <cmath>
#include <iomanip>

int main() {
    double a, b, c;
    std::cout << "Введите коэффициенты a, b и c (через пробел): ";
    if (!(std::cin >> a >> b >> c)) {
        std::cerr << "Ошибка ввода\n";
        return 1;
    }

    if (a == 0.0) {
        std::cerr << "Это не квадратное уравнение (a == 0)\n";
        return 1;
    }

    double D = b * b - 4.0 * a * c;
    const double eps = 1e-14;  // для сравнения с нулем

    if (D > eps) {
        double sqrtD = std::sqrt(D);
        double x1 = (-b + sqrtD) / (2.0 * a);
        double x2 = (-b - sqrtD) / (2.0 * a);
        std::cout << std::fixed << std::setprecision(6)
                  << "Два действительных корня:\n"
                  << "x1 = " << x1 << "\n"
                  << "x2 = " << x2 << "\n";
    }
    else if (std::fabs(D) <= eps) {
        double x = -b / (2.0 * a);
        std::cout << std::fixed << std::setprecision(6)
                  << "Один действительный корень:\n"
                  << "x = " << x << "\n";
    }
    else {
        std::cout << "Действительных корней нет\n";
    }

    return 0;
}
