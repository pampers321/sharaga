#include <thread>
#include <windows.h>
#include <clocale>
#include <iostream>
#include "TimerGuard.h"

int main() {
    {
        SetConsoleOutputCP(CP_UTF8);
        int g = 0;
        TimerGuard t("цикл");
        for (volatile int i = 0; i < 10000000; ++i) {
            g++;
         }
    }

    {
        TimerGuard t("sleep 150 ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    return 0;
}