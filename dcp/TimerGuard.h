#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

class TimerGuard {
    std::string m;
    std::ostream& o;
    std::chrono::steady_clock::time_point s;
public:
    TimerGuard(std::string_view msg = "", std::ostream& out = std::cout)
        : m(msg), o(out), s(std::chrono::steady_clock::now()) {}

    ~TimerGuard() {
        o << m << ' '
          << std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::steady_clock::now() - s).count()
          << '\n';
    }
};
