#include "HttpServer.h"

#include "DataEngine.h"
#include "Logger.h"

#ifdef _MSC_VER
#  include <SDKDDKVer.h>
#  pragma warning( push )
#  pragma warning( disable : 4267 ) // warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4244 ) // warning C4244: '=': conversion from '__int64' to 'unsigned long', possible loss of data
#endif
// =====================
#    include "crow/app.h"
#    include "crow/logging.h"
// =====================
#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include <thread> // for std::thread::hardware_concurrency()


namespace
{
    class LogHandler : public crow::ILogHandler
    {
    public:
        virtual void log(std::string message, crow::LogLevel level) override
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
    };

    LogHandler g_logger;
}


class HttpServerApp : public crow::SimpleApp
{
};


HttpServer::HttpServer():
    m_ptrApp(std::make_unique<HttpServerApp>())
{
}

HttpServer::~HttpServer() = default;


void HttpServer::run(const std::string& host, const std::uint16_t port, const DataEngine& engine)
{
    LOG_INFO << "HttpServer: run: begin" << std::endl;

    // Setup global Crow logger:
    crow::logger::setHandler(&g_logger);

    m_ptrApp->loglevel(crow::LogLevel::Info);

    setup_routing(engine);

    m_ptrApp->bindaddr(host).port(port);

    m_ptrApp->concurrency(std::thread::hardware_concurrency());

    m_ptrApp->run();

    LOG_INFO << "HttpServer: run: end" << std::endl;
}

void HttpServer::setup_routing(const DataEngine& engine)
{
    crow::SimpleApp& app = *m_ptrApp;

    CROW_ROUTE(app, "/")([]() {
        return "Hello world";
        });
}

void HttpServer::stop_notify()
{
    LOG_INFO << "HttpServer: stop_notify: begin" << std::endl;
    m_ptrApp->stop();
    LOG_INFO << "HttpServer: stop_notify: end" << std::endl;
}
