#include <chrono>
#include <iostream>
#include <cstdint>
#include <cmath>

using result_t = int64_t;

result_t computePartA(result_t result) {
    return result << std::min(result_t(32), std::max(result_t(1), std::abs(result + 1)));
}

result_t computePartB(result_t result) {
    return std::round(std::sqrt(std::max(result_t(1), result)));
}

result_t computePartC(result_t result) {
    return std::log(std::max(result_t(1), result));
}

result_t compute(result_t result) {
    for (size_t i = 0; i < 4096; ++i) {
        result = computePartA(result);
        result = computePartB(result);
        result = computePartC(result);
    }

    return result;
}

int main(int argc, char** argv) {
    std::cout << "Starting test program." << std::endl;

    const auto delay = argc > 1 ? atoi(argv[1]) : 3;

    const auto startTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> currentTime;

    int32_t result = 0;
    uint64_t iterations = 0;

    do {
        result = compute(result);
        iterations += 1;
        currentTime = std::chrono::steady_clock::now();
    } while((currentTime - startTime) < std::chrono::seconds(delay));

    std::cout << "Result: " << result << std::endl;
    std::cout << "Completed " << iterations << " iterations." << std::endl;
    std::cout << "Ending test program." << std::endl;
}
