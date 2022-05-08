#include "BackgroundWorker.h"

#include "DataEngine.h"
#include "DataSerializer.h"
#include "Logger.h"
#include "StatisticsPrinter.h"

#include <chrono>


void BackgroundWorker::run(const DataEngine& engine, const std::string& databaseFilename)
{
    LOG_INFO << "BackgroundWorker: begin" << std::endl;

    using namespace std::chrono_literals;
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    constexpr auto timeBetweenStatsPrints = 5s;
    constexpr auto timeBetweenDataSaves = 14s;
    static_assert(timeBetweenDataSaves >= timeBetweenStatsPrints);

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
            const size_t savedRecordCount = store_data(engine, databaseFilename);
            LOG_INFO << "BackgroundWorker: saved " << savedRecordCount << " DB records to file " << databaseFilename << std::endl;
            lastSaveTime = saveTime;
        }

        {
            std::unique_lock lock(m_protect);
            const bool terminating = m_conditional.wait_until(
                lock,
                printTime + timeBetweenStatsPrints,
                [this] { return m_terminating; }
            );
            if (terminating)
            {
                break;
            }
        }
    }

    LOG_INFO << "BackgroundWorker: end" << std::endl;
}

void BackgroundWorker::stop_notify()
{
    LOG_INFO << "BackgroundWorker: stop_notify: begin" << std::endl;

    {
        const std::lock_guard lock(m_protect);
        m_terminating = true;
    }
    m_conditional.notify_all();

    LOG_INFO << "BackgroundWorker: stop_notify: end" << std::endl;
}

size_t BackgroundWorker::initial_load_data(DataEngine& engine, const std::string& databaseFilename)
{
    size_t recordCount = 0;
    auto loadVisitor = [&engine, &recordCount](const std::string_view name, const std::string_view value)
    {
        engine.initial_set(name, value);
        ++recordCount;
        return;
    };

    const bool ok = DataSerializer::load(databaseFilename, loadVisitor);
    if (!ok)
    {
        LOG_ERROR << "DataSerializer::load() failed" << std::endl;
        return recordCount;
    }

    return recordCount;
}

size_t BackgroundWorker::store_data(const DataEngine& engine, const std::string& databaseFilename)
{
    size_t recordCount = 0;
    DataSerializer::Document document;

    const std::function<DataEngine::EnumerateVisitorProc> visitor =
        [&document, &recordCount](const std::string_view name, const std::string_view value)
    {
        document.add(name, value);
        ++recordCount;
        return;
    };

    engine.enumerate(visitor);

    const bool ok = DataSerializer::save(databaseFilename, document);
    if (!ok)
    {
        LOG_ERROR << "DataSerializer::save() failed" << std::endl;
        return recordCount;
    }

    return recordCount;
}
