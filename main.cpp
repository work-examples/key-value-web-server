#include "BackgroundWorker.h"
#include "DataEngine.h"
#include "HttpServer.h"
#include "Logger.h"

#include <cstdint>
#include <future>
#include <string>
#include <string_view>
#include <thread>


int main(const int argc, const char* const* const argv)
{
    const Logger::LogLevel logLevel = Logger::LogLevel::Debug;
    Logger::SetLogLevel(logLevel);

    LOG_INFO << "main: begin" << std::endl;
    const std::string arg1 = argc > 1 ? argv[1] : "";

    // =========================================================
    // ===   Configuration:
    // =========================================================

    const std::string   listenHost = "127.0.0.1";
    const std::uint16_t listenPort = 8000;
    const std::string   databaseFilename = "database.json";
    const bool          logEachRequest = arg1 != "--no-logs";

    // =========================================================

    DataEngine engine;

    LOG_INFO << "main: load data" << std::endl;
    const size_t loadedRecordCount = BackgroundWorker::initial_load_data(engine, databaseFilename);
    LOG_INFO << "main: loaded " << loadedRecordCount << " DB records from file " << databaseFilename << std::endl;

    {
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
    }

    LOG_INFO << "main: save data to file before process exit" << std::endl;
    const size_t savedRecordCount = BackgroundWorker::store_data(engine, databaseFilename);
    LOG_INFO << "main: saved " << savedRecordCount << " DB records to file " << databaseFilename << std::endl;

    LOG_INFO << "main: end" << std::endl;

    return 0;
}
