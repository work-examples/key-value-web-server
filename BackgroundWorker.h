#pragma once

#include <mutex>
#include <string>


class DataEngine;


class BackgroundWorker
{
public:
    void run(const DataEngine& engine, const std::string& databaseFilename);

    void stop();

protected:
    std::mutex              m_protect;
    std::condition_variable m_conditional;
    bool                    m_terminating = false;
};
