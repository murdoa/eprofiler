#include <cstdlib>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

struct SteadyClock {
    using time_point = int;

    static time_point now() {
        return rand();
    }
};

int app(int argc, char *argv[]) {
    using EProfiler = eprofiler::EProfiler<eprofiler::EProfilerTag{"Test"}, int, SteadyClock>;

    EProfiler::set_time("Tag1"_sc);
    EProfiler::set_time("Tag2"_sc);
    EProfiler::set_time("Tag3"_sc);

    return EProfiler::get_duration("Tag1"_sc, "Tag3"_sc);
}