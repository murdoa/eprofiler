// #include <iostream>

#include <eprofiler/eprofiler.hpp>

#include <eprofiler/linktimehashtable.hpp>

using namespace eprofiler::literals;

int app(int argc, char *argv[]) {
    const auto profiler = eprofiler::EProfiler<"Profiler", int, int>{};

    profiler["Test"_sc] = 5;

    return profiler["Test"_sc];
}