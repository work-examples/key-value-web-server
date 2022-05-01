#include "BackgroundWorker.h"

#include "DataEngine.h"
#include "DataSerializer.h"
#include "Logger.h"
#include "StatisticsPrinter.h"

#include <chrono>


void BackgroundWorker::run(const DataEngine& engine, const std::string& databaseFilename)
{
    using namespace std::chrono_literals;
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    constexpr auto timeBetweenStatsPrints = 5s;
    constexpr auto timeBetweenDataSaves = 14s;
    static_assert(timeBetweenDataSaves >= timeBetweenStatsPrints);

    LOG_INFO << "BackgroundWorker: started" << std::endl;

    StatisticsPrinter printer;

    auto lastPrintTime = clock::now();
    auto lastSaveTime = lastPrintTime;

    while (true)
    {
        const auto printTime = clock::now();
        {
            const auto printSecondsElapsed = std::chrono::duration_cast<std::chrono::seconds>(printTime - lastPrintTime);
            const auto stats = engine.get_global_statistics();
            printer.print_statistics(stats.m_reads, stats.m_writes, static_cast<unsigned>(printSecondsElapsed.count()));
            lastPrintTime = printTime;
        }

        const auto saveTime = clock::now();
        if (saveTime - lastSaveTime >= timeBetweenDataSaves)
        {
            const auto saveSecondsElapsed = std::chrono::duration_cast<std::chrono::seconds>(saveTime - lastSaveTime);
            LOG_INFO << "BackgroundWorker: save data to file after " << saveSecondsElapsed.count() << " seconds" << std::endl;

            DataSerializer::Document document;

            const std::function<DataEngine::EnumerateVisitorProc> visitor =
                [&document](const std::string_view name, const std::string_view value)
            {
                document.add(name, value);
                return;
            };
            engine.enumerate(visitor);

            DataSerializer::save(databaseFilename, document);
            lastSaveTime = saveTime;
        }

        {
            std::unique_lock lock(m_protect);
            const bool terminating = m_conditional.wait_until(lock, printTime + timeBetweenStatsPrints, [this] { return m_terminating; });
            if (terminating)
            {
                break;
            }
        }
    }

    LOG_INFO << "BackgroundWorker: exit" << std::endl;
}

void BackgroundWorker::stop()
{
    LOG_INFO << "BackgroundWorker: stop" << std::endl;
    {
        const std::lock_guard lock(m_protect);
        m_terminating = true;
    }
    m_conditional.notify_all();
}