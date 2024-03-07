// #include <iostream>

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include <eprofiler/eprofiler.hpp>
#include <eprofiler/linktimehashtable.hpp>

using namespace eprofiler::literals;

int app(int argc, char *argv[]) {
    const auto profiler = eprofiler::EProfiler<"Profiler", std::size_t, std::chrono::steady_clock::time_point>{};

    profiler["Inital_Tag"_sc] = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    profiler["Tag1"_sc] = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    profiler["Tag2"_sc] = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    profiler["Tag3"_sc] = std::chrono::steady_clock::now();

    spdlog::info("Profiler name: {}", profiler.name());
    spdlog::info("Profiler offset: {}", profiler.get_offset());

    spdlog::info("Inital_Tag: {}", profiler["Inital_Tag"_sc].time_since_epoch().count());
    spdlog::info("Tag1: {}", std::chrono::duration_cast<std::chrono::microseconds>(profiler["Tag1"_sc] - profiler["Inital_Tag"_sc]).count());
    spdlog::info("Tag2: {}", std::chrono::duration_cast<std::chrono::microseconds>(profiler["Tag2"_sc] - profiler["Inital_Tag"_sc]).count());
    spdlog::info("Tag3: {}", std::chrono::duration_cast<std::chrono::microseconds>(profiler["Tag3"_sc] - profiler["Inital_Tag"_sc]).count());


    // const auto profiler = eprofiler::EProfiler<"Profiler", int, int>{};

    // profiler["Inital_Tag"_sc] = 1;
    // return profiler["Inital_Tag"_sc];

    return 0;
}