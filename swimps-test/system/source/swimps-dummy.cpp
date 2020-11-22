#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Starting test program." << std::endl;

    const auto delay = argc > 1 ? atoi(argv[1]) : 3;

    const auto startTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> currentTime;

    do {
        currentTime = std::chrono::steady_clock::now();
    } while((currentTime - startTime) < std::chrono::seconds(delay));

    std::cout << "Ending test program." << std::endl;
}
