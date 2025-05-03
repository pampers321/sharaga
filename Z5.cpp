#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <iomanip>

static const uint32_t BASE = 1000000000; // 10^9

// Класс для работы с большими неотрицательными числами
class BigInt {
public:
    // младший чанк — в front(), старший — в back()
    std::list<uint32_t> chunks;
    bool negative = false;

    BigInt() = default;

    // Парсинг из строки (только цифры, до 200 символов)
    static BigInt fromString(const std::string &s) {
        if (s.empty() || s.size() > 200)
            throw std::invalid_argument("Неверная длина числа");
        for (char c : s)
            if (!std::isdigit(c))
                throw std::invalid_argument("Неверный символ в числе");

        BigInt R;
        int len = int(s.size());
        for (int i = len; i > 0; i -= 9) {
            int start = std::max(0, i - 9);
            int count = i - start;
            uint32_t chunk = std::stoul(s.substr(start, count));
            R.chunks.push_back(chunk);
        }
        R.trim();
        return R;
    }

    // Удаляем ведущие нулевые чанки (верхний край), оставляем хотя бы один
    void trim() {
        while (chunks.size() > 1 && chunks.back() == 0)
            chunks.pop_back();
        if (chunks.size() == 1 && chunks.front() == 0)
            negative = false;
    }

    // Сравнение по абсолютному значению
    static int cmpAbs(const BigInt &A, const BigInt &B) {
        if (A.chunks.size() != B.chunks.size())
            return A.chunks.size() < B.chunks.size() ? -1 : +1;
        // одинаковая длина — сравниваем от старшего чанка
        auto a_it = A.chunks.rbegin();
        auto b_it = B.chunks.rbegin();
        while (a_it != A.chunks.rend()) {
            if (*a_it != *b_it)
                return *a_it < *b_it ? -1 : +1;
            ++a_it; ++b_it;
        }
        return 0;
    }

    // Сложение абсолютных значений
    static BigInt addAbs(const BigInt &A, const BigInt &B) {
        BigInt R;
        auto itA = A.chunks.begin();
        auto itB = B.chunks.begin();
        uint64_t carry = 0;
        while (itA != A.chunks.end() || itB != B.chunks.end() || carry) {
            uint64_t a = (itA != A.chunks.end() ? *itA++ : 0);
            uint64_t b = (itB != B.chunks.end() ? *itB++ : 0);
            uint64_t sum = a + b + carry;
            R.chunks.push_back(uint32_t(sum % BASE));
            carry = sum / BASE;
        }
        return R;
    }

    // Вычитание абсолютных значений: предполагаем A >= B
    static BigInt subAbs(const BigInt &A, const BigInt &B) {
        BigInt R;
        auto itA = A.chunks.begin();
        auto itB = B.chunks.begin();
        int64_t borrow = 0;
        while (itA != A.chunks.end()) {
            int64_t a = int64_t(*itA++) - borrow;
            int64_t b = (itB != B.chunks.end() ? *itB++ : 0);
            if (a < b) {
                a += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            R.chunks.push_back(uint32_t(a - b));
        }
        R.trim();
        return R;
    }

    // Операция сложения с учётом знака
    BigInt operator+(const BigInt &other) const {
        if (negative == other.negative) {
            BigInt R = addAbs(*this, other);
            R.negative = negative;
            R.trim();
            return R;
        } else {
            // A + (-B) = A - B
            if (cmpAbs(*this, other) >= 0) {
                BigInt R = subAbs(*this, other);
                R.negative = negative;
                R.trim();
                return R;
            } else {
                BigInt R = subAbs(other, *this);
                R.negative = other.negative;
                R.trim();
                return R;
            }
        }
    }

    // Вычитание с учётом знака
    BigInt operator-(const BigInt &other) const {
        if (negative != other.negative) {
            // A - (-B) = A + B
            BigInt R = addAbs(*this, other);
            R.negative = negative;
            R.trim();
            return R;
        } else {
            // A - B = ?
            if (cmpAbs(*this, other) >= 0) {
                BigInt R = subAbs(*this, other);
                R.negative = negative;
                R.trim();
                return R;
            } else {
                BigInt R = subAbs(other, *this);
                R.negative = !negative;
                R.trim();
                return R;
            }
        }
    }

    // Умножение методом «столбиком»
    BigInt operator*(const BigInt &other) const {
        // переводим в вектор для удобного индексирования
        std::vector<uint64_t> a(chunks.begin(), chunks.end());
        std::vector<uint64_t> b(other.chunks.begin(), other.chunks.end());
        std::vector<uint64_t> res(a.size() + b.size(), 0);

        for (size_t i = 0; i < a.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < b.size() || carry; ++j) {
                uint64_t cur = res[i + j] +
                    a[i] * (j < b.size() ? b[j] : 0) +
                    carry;
                res[i + j] = cur % BASE;
                carry = cur / BASE;
            }
        }
        BigInt R;
        R.negative = (negative != other.negative);
        // переносим ненулевые части в список
        size_t k = res.size();
        while (k > 1 && res[k-1] == 0) --k;
        for (size_t i = 0; i < k; ++i)
            R.chunks.push_back(uint32_t(res[i]));
        R.trim();
        return R;
    }

    // Преобразование обратно в десятичную строку
    std::string toString() const {
        if (chunks.empty()) return "0";
        std::string s;
        if (negative) s.push_back('-');
        // самый старший чанк без ведущих нулей
        auto it = chunks.rbegin();
        s += std::to_string(*it++);
        // остальные — ровно 9 цифр с ведущими нулями
        for (; it != chunks.rend(); ++it) {
            std::ostringstream oss;
            oss << std::setw(9) << std::setfill('0') << *it;
            s += oss.str();
        }
        return s;
    }
};

int main() {
    try {
        std::string sa, sb;
        std::cout << "Введите два неотрицательных целых числа (до 200 цифр) через пробел:\n> ";
        if (!(std::cin >> sa >> sb)) {
            std::cerr << "Ошибка ввода\n";
            return 1;
        }
        BigInt A = BigInt::fromString(sa);
        BigInt B = BigInt::fromString(sb);

        BigInt sum = A + B;
        BigInt diff = A - B;
        BigInt prod = A * B;

        std::cout << "\nРезультаты:\n";
        std::cout << "  Сумма       : " << sum.toString()  << "\n";
        std::cout << "  Разность    : " << diff.toString() << "\n";
        std::cout << "  Произведение: " << prod.toString() << "\n";
    }
    catch (const std::exception &ex) {
        std::cerr << "Ошибка: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
