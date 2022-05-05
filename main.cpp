#include "BackgroundWorker.h"
#include "DataEngine.h"
#include "DataSerializer.h"
#include "HttpServer.h"
#include "Logger.h"

#include <cstdint>
#include <future>
#include <string>
#include <string_view>
#include <thread>


int main(void)
{
    LOG_INFO << "main: begin" << std::endl;

    // =========================================================
    // ===   Configuration:
    // =========================================================

    const std::string   listenHost = "127.0.0.1";
    const std::uint16_t listenPort = 8000;
    const std::string databaseFilename = "database.json";
    const bool logEachRequest = true;

    // =========================================================

    DataEngine engine;

    LOG_INFO << "main: load data" << std::endl;

    {
        size_t recordCount = 0;
        auto loadVisitor = [&engine, &recordCount](const std::string_view name, const std::string_view value)
        {
            engine.initial_set(name, value);
            ++recordCount;
        };
        DataSerializer::load(databaseFilename, loadVisitor);
        LOG_INFO << "main: loaded " << recordCount << " DB records from file " << databaseFilename << std::endl;
    }

    BackgroundWorker worker;

    auto backgroundWorkerProc = [&worker, &engine, &databaseFilename]()
    {
        worker.run(engine, databaseFilename);
    };

    auto backgroundWorkerFuture = std::async(std::launch::async, backgroundWorkerProc);

    LOG_INFO << "main: listening connections: begin" << std::endl;

    HttpServer server;
    server.run(listenHost, listenPort, engine, logEachRequest);

    LOG_INFO << "main: listening connections: end" << std::endl;

    worker.stop_notify();
    backgroundWorkerFuture.wait();

    LOG_INFO << "main: end" << std::endl;

    return 0;
}
