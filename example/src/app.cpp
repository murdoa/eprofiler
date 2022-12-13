#include <iostream>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;


int app(int argc, char *argv[]) {
    const auto profiler = EPROFILER_GET_PROFILER("Profiler");

    std::cout << "ID: " << profiler["Test"_sc] << "\n";
    std::cout << "ID: " << profiler["Test1"_sc] << "\n";
    std::cout << "ID: " << profiler["Test2"_sc] << "\n";
    std::cout << "ID: " << profiler["Test3"_sc] << "\n";

    for (auto& tag : profiler.tags) {
        std::cout << "Tag: " << tag << "\n";
    }

    return 0;
}