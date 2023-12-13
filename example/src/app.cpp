#include <iostream>

#include <eprofiler/eprofiler.hpp>

#include <eprofiler/linktimehashtable.hpp>

using namespace eprofiler::literals;


int app(int argc, char *argv[]) {



    const auto profiler = eprofiler::EProfiler<"Profiler", int>{};

    std::cout << "ID: " << profiler.get_id("Test"_sc) << "\n";
    std::cout << "ID: " << profiler.get_id("Test1"_sc) << "\n";
    std::cout << "ID: " << profiler.get_id("Test2"_sc) << "\n";
    std::cout << "ID: " << profiler.get_id("Test3"_sc) << "\n";


    return 0;
}