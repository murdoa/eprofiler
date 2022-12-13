#include <iostream>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;


int app(int argc, char *argv[]) {
    const auto profiler = EPROFILER_GET_PROFILER("Profiler");

    std::cout << "\"Test\" ID: " << profiler["Test"_sc] << "\n";

    return 0;
}