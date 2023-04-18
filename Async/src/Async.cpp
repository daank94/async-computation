// Async.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <functional>
#include <array>

#include "../include/ThreadPool.h"
#include "../include/Promise.h"

int main()
{
    std::cout << "Hello World!" << std::endl;

    ThreadPool tp;

    std::function<int()> fun_to_run = []() {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        return 1;
    };

    Promise<int> p = tp.runAsync(fun_to_run);
    // We have the main thread free, do something in the meantime.
    const std::array<const char*, 11> string_to_print = { "The", "main", "loop", "can", "do", "all", "kinds", "of", "things", "right", "now" };
    for (const char* str : string_to_print) {
        std::cout << str << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "The promise will now block until the result is ready" << std::endl;
    tp.stop();
    std::cout << p.getValue() << std::endl;
}
