#pragma once

#include <deque>
#include <cstdint>

// TODO: this struct shouldn't be here
struct SKeyScan {
    SKeyScan() = default;

    SKeyScan(uint32_t _time, uint32_t _scan)
    : time{_time}
    , scan{_scan}
    {}

    uint32_t time;
    uint32_t scan;
};

bool processCheats(std::deque<SKeyScan>& input);