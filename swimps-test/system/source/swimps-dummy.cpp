#include <chrono>
#include <iostream>

int main() {
    std::cout << "Starting test program." << std::endl;

    const auto startTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> currentTime;

    do {
        currentTime = std::chrono::steady_clock::now();
    } while((currentTime - startTime) < std::chrono::seconds(3));

    std::cout << "Ending test program." << std::endl;
}
