#pragma once

#include <cstdint>


class StatisticsPrinter
{
public:
    using IntegerCounter = std::uint64_t;

    void print_statistics(const IntegerCounter reads, const IntegerCounter writes, const unsigned secondsElapsed);

protected:
    IntegerCounter m_previousReads = 0;
    IntegerCounter m_previousWrites = 0;
};
