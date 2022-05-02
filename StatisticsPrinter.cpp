#include "StatisticsPrinter.h"

#include "Logger.h"


void StatisticsPrinter::print_statistics(const IntegerCounter reads, const IntegerCounter writes, const unsigned secondsElapsed)
{
    const IntegerCounter readsDelta = reads - m_previousReads;
    const IntegerCounter writesDelta = writes - m_previousWrites;

    LOG_INFO << "Statistics:"
        << " total: { reads = " << reads << "; writes = " << writes << " }"
        << " last " << secondsElapsed << " seconds: { reads = " << readsDelta << "; writes = " << writesDelta << " }"
        << std::endl;

    m_previousReads = reads;
    m_previousWrites = writes;
}
