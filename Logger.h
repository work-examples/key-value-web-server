#pragma once

#include <iomanip>
#include <ios>
#include <mutex>
#include <sstream>
#include <string>


class Logger
{
public:
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Critical,
    };

    static void log(const std::string& message, const LogLevel logLevel);

protected:
    static std::mutex s_protect;
};

class LogStream
{
public:
    LogStream(const Logger::LogLevel logLevel) :
        m_logLevel(logLevel)
    {
    }

    ~LogStream()
    {
        log(); // automatic flush
    }

    template<typename T>
    LogStream& operator<<(T const& value)
    {
        m_buffer << value;
        m_isEmptyBuffer = false;
        return *this;
    }

    template<typename T>
    LogStream& operator<<(const T* value)
    {
        m_buffer << value;
        m_isEmptyBuffer = false;
        return *this;
    }

    LogStream& operator <<(decltype(std::endl<char, std::char_traits<char>>) value)
    {
        log(); // flush
        return *this;
    }

    LogStream& operator <<(decltype(std::hex) value)
    {
        m_buffer << value;
        return *this;
    }

    LogStream& operator <<(decltype(std::setw) value)
    {
        m_buffer << value;
        return *this;
    }

private:
    void log()
    {
        if (!m_isEmptyBuffer)
        {
            Logger::log(m_buffer.str(), m_logLevel);
            m_buffer.str(""); // clear std::stringstream content
            m_isEmptyBuffer = true;
        }
    }

    std::ostringstream m_buffer;
    bool               m_isEmptyBuffer = true;
    Logger::LogLevel   m_logLevel = Logger::LogLevel::Critical;
};


#define LOG_INFO  LogStream(Logger::LogLevel::Info)
#define LOG_ERROR LogStream(Logger::LogLevel::Error)
