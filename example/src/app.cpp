#include <cstdlib>

// #include <chrono>
// #include <thread>
// #include <spdlog/spdlog.h>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

struct SteadyClock {
    using time_point = int;
    using duration = int;

    static time_point now() {
        return rand();
    }
};

int app(int argc, char *argv[]) {
    using EProfiler = eprofiler::EProfiler<eprofiler::EProfilerTag{"Test"}, int, SteadyClock>;//std::chrono::steady_clock>;

    EProfiler::set_time("Tag1"_sc);
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EProfiler::set_time("Tag1"_sc, "Tag2"_sc);
    // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EProfiler::set_time("Tag3"_sc);

    const auto duration = EProfiler::get_duration("Tag1"_sc, "Tag3"_sc);

    // spdlog::info("Duration: {}", std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());

    // const auto& value_store = EProfiler::value_store();
    // for (std::size_t i = 0; i < value_store.size(); ++i) {
    //     const auto& tag = EProfiler::keys()[i];
    //     const auto& [index, time] = value_store[i];

    //     if (index.has_value()) {
    //         spdlog::info("Tag: {}, Duration: {}", tag, EProfiler::get_duration(i + EProfiler::offset()).count());
    //     } else {
    //         spdlog::info("Tag: {}, Time: {}", tag, time.time_since_epoch().count());
    //     }
    // }

    return 0;
}