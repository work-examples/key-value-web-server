#pragma once

#include <condition_variable>
#include <mutex>
#include <string>


class DataEngine;


class BackgroundWorker
{
public:
    void run(const DataEngine& engine, const std::string& databaseFilename);

    void stop_notify();

public:
    static size_t initial_load_data(DataEngine& engine, const std::string& databaseFilename);
    static size_t store_data(const DataEngine& engine, const std::string& databaseFilename);

protected:
    std::mutex              m_protect;
    std::condition_variable m_conditional;
    bool                    m_terminating = false;
};
