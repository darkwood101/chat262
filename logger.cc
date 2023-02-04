#include "logger.h"

#include <cinttypes>
#include <sstream>
#include <thread>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::time_point;

static const time_point<high_resolution_clock> start_of_time =
    high_resolution_clock::now();

void logger::add_prefix(FILE* out) {
    fprintf(out, "[");

    // Print thread id
    std::stringstream ss;
    ss << "T-0x" << std::hex << std::this_thread::get_id();
    fprintf(out, "%s", ss.str().c_str());

    fprintf(out, " | ");

    // Print timestamp
    time_point<high_resolution_clock> now = high_resolution_clock::now();
    microseconds dur_us = duration_cast<microseconds>(now - start_of_time);
    fprintf(out, "%" PRId64 "us", dur_us.count());

    fprintf(out, "] ");
}
