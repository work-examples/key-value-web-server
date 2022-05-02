#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead.
#endif

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

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>

#include <cstdio>
#include <string_view>
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

    std::string url_decode(const std::string& value)
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

    class JsonBody: public crow::returnable
    {
    public:
        JsonBody() :
            crow::returnable("application/json; charset=utf-8"),
            m_document(rapidjson::Type::kObjectType)
        {
        }

        void add(const std::string_view name, const std::string_view value)
        {
            const rapidjson::Document::StringRefType nameRef(name.data(), static_cast<rapidjson::SizeType>(name.size()));
            const rapidjson::Document::StringRefType valueRef(value.data(), static_cast<rapidjson::SizeType>(value.size()));
            auto& allocator = m_document.GetAllocator();
            m_document.AddMember(nameRef, valueRef, allocator);
        }

        void add(const std::string_view name, const std::string& value)
        {
            const rapidjson::Document::StringRefType nameRef(name.data(), static_cast<rapidjson::SizeType>(name.size()));
            const rapidjson::Document::StringRefType valueRef(value.data(), static_cast<rapidjson::SizeType>(value.size()));
            auto& allocator = m_document.GetAllocator();
            m_document.AddMember(nameRef, valueRef, allocator);
        }

        template<typename T>
        void add(const std::string_view name, const T value)
        {
            const rapidjson::Document::StringRefType nameRef(name.data(), static_cast<rapidjson::SizeType>(name.size()));
            auto& allocator = m_document.GetAllocator();
            m_document.AddMember(nameRef, value, allocator);
        }

        virtual std::string dump() const override
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            m_document.Accept(writer);
            return { buffer.GetString() , buffer.GetLength() };
        }

    protected:
        rapidjson::Document m_document;
    };
}


class HttpServerApp : public crow::SimpleApp
{
};


HttpServer::HttpServer():
    m_ptrApp(std::make_unique<HttpServerApp>())
{
}

HttpServer::~HttpServer() = default;


void HttpServer::run(const std::string& host, const std::uint16_t port, DataEngine& engine)
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

void HttpServer::stop_notify()
{
    LOG_INFO << "HttpServer: stop_notify: begin" << std::endl;
    m_ptrApp->stop();
    LOG_INFO << "HttpServer: stop_notify: end" << std::endl;
}

void HttpServer::setup_routing(DataEngine& engine)
{
    crow::SimpleApp& app = *m_ptrApp;

    // Get value
    CROW_ROUTE(app, "/api/records/<string>").methods(crow::HTTPMethod::GET)(
        [&engine](const crow::request& req, const std::string& nameRaw)
        {
            using namespace std::literals;
            JsonBody body;
            try
            {
                const std::string name = url_decode(nameRaw);
                body.add("name"sv, name);

                if (name.empty())
                {
                    body.add("error"sv, "Item name cannot be empty"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                const std::optional<DataEngine::ExtendedValue> value = engine.get(name);
                if (!value.has_value())
                {
                    body.add("error"sv, "Item not found"sv);
                    return crow::response(crow::status::NOT_FOUND, body);
                }

                body.add("value"sv, value->m_value);
                body.add("stats.reads"sv, value->m_stats.m_reads);
                body.add("stats.writes"sv, value->m_stats.m_writes);
                return crow::response(crow::status::OK, body);
            }
            catch (...)
            {
                body.add("error"sv, "Server internal error"sv);
                return crow::response(crow::status::INTERNAL_SERVER_ERROR, body);
            }
        }
    );

    // Set value
    CROW_ROUTE(app, "/api/records/<string>").methods(crow::HTTPMethod::PATCH)(
        [&engine](const crow::request& req,  const std::string& nameRaw)
        {
            using namespace std::literals;
            JsonBody body;
            try
            {
                const std::string name = url_decode(nameRaw);
                body.add("name"sv, name);

                if (name.empty())
                {
                    body.add("error"sv, "Item name cannot be empty"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                rapidjson::StringStream requestStream(req.body.c_str());
                rapidjson::Document requestDocument;
                requestDocument.ParseStream(requestStream);

                if (requestDocument.HasParseError())
                {
                    body.add("error"sv, "Invalid JSON syntax in request body"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                if (!requestDocument.IsObject())
                {
                    body.add("error"sv, "Request body JSON must be an Object"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                const auto& value = requestDocument["value"];
                engine.set(name, std::string_view(value.GetString(), value.GetStringLength()));

                return crow::response(crow::status::OK, body);
            }
            catch (...)
            {
                body.add("error"sv, "Server internal error"sv);
                return crow::response(crow::status::INTERNAL_SERVER_ERROR, body);
            }
        }
    );

    CROW_ROUTE(app, "/api/records/")(
        []()
        {
            return crow::response(crow::status::NOT_IMPLEMENTED, "txt", "Item listing is not supported");
        }
    );

    CROW_ROUTE(app, "/")(
        []()
        {
            return crow::response(crow::status::OK, "txt", "Hello, World!\n");
        }
    );
}
