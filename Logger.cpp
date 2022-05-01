#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string_view>


std::mutex Logger::s_protect;


void Logger::log(const std::string& message, const LogLevel logLevel)
{
    using namespace std::literals;

    std::string_view strLogLevel = "UNKNOWN"sv;
    switch (logLevel)
    {
    case LogLevel::Debug:
        strLogLevel = "DEBUG"sv;
        break;
    case LogLevel::Info:
        strLogLevel = "INFO"sv;
        break;
    case LogLevel::Warning:
        strLogLevel = "WARNING"sv;
        break;
    case LogLevel::Error:
        strLogLevel = "ERROR"sv;
        break;
    case LogLevel::Critical:
        strLogLevel = "CRITICAL"sv;
        break;
    }

    auto& stream = logLevel >= LogLevel::Warning ? std::cerr : std::cout;

    const auto now = std::chrono::system_clock::now();
    const unsigned milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    const std::time_t tNow = std::chrono::system_clock::to_time_t(now);
    tm tmNow = {};

#if defined(_MSC_VER) || defined(__MINGW32__)
    gmtime_s(&tmNow, &tNow);
#else
    gmtime_r(&tNow, &tmNow);
#endif

    std::lock_guard lock(s_protect);

    stream
        << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << std::right << milliseconds
        << ' ' << std::setw(8) << std::setfill(' ') << std::left << strLogLevel
        << ' ' << message
        << std::endl;
}
