#include "HttpServerHelpers.h"

#include "Logger.h"

#include <cstdio>


#ifdef _MSC_VER
#pragma warning( disable : 4996 ) // warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead.
#endif


std::string HttpServerHelpers::url_decode(const std::string& value)
{
    const size_t len = value.length();

    std::string result;
    result.reserve(len);

    for (size_t i = 0; i < len; ++i)
    {
        if (value[i] == '+')
        {
            result += ' ';
            continue;
        }
        if (value[i] == '%' && i < len - 2)
        {
            char buf[] = { value[i + 1], value[i + 2], '\0' };
            int number = 0;
            if (EOF != std::sscanf(buf, "%x", &number))
            {
                result += static_cast<char>(number);
                i += 2;
                continue;
            }
        }
        result += value[i];
    }
    return result;
}

void HttpServerHelpers::LogHandler::log(std::string message, crow::LogLevel level)
{
    Logger::LogLevel newLevel = Logger::LogLevel::Critical;
    switch (level)
    {
    case crow::LogLevel::Debug:
        newLevel = Logger::LogLevel::Debug;
        break;
    case crow::LogLevel::Info:
        newLevel = Logger::LogLevel::Info;
        break;
    case crow::LogLevel::Warning:
        newLevel = Logger::LogLevel::Warning;
        break;
    case crow::LogLevel::Error:
        newLevel = Logger::LogLevel::Error;
        break;
    case crow::LogLevel::Critical:
        newLevel = Logger::LogLevel::Critical;
        break;
    }

    Logger::log(message, newLevel);
}
