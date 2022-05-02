#include "HttpServer.h"

#include "DataEngine.h"
#include "HttpServerHelpers.h"
#include "Logger.h"

#ifdef _MSC_VER
#  include <SDKDDKVer.h>
#  pragma warning( push )
#  pragma warning( disable : 4267 ) // warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4244 ) // warning C4244: '=': conversion from '__int64' to 'unsigned long', possible loss of data
#endif
// =====================
#    include "crow/app.h"
// =====================
#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include "rapidjson/document.h"
#include "rapidjson/stream.h"


#include <thread> // for std::thread::hardware_concurrency()


namespace
{
    HttpServerHelpers::LogHandler g_logger;
}


class HttpServerApp : public crow::SimpleApp
{
};


HttpServer::HttpServer():
    m_ptrApp(std::make_unique<HttpServerApp>())
{
}

HttpServer::~HttpServer() = default;


void HttpServer::run(const std::string& host, const std::uint16_t port, DataEngine& engine, const bool logEachRequest)
{
    LOG_INFO << "HttpServer: run: begin" << std::endl;

    // Setup global Crow logger:
    crow::logger::setHandler(&g_logger);

    m_ptrApp->loglevel(logEachRequest ? crow::LogLevel::Info : crow::LogLevel::Warning);

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
            HttpServerHelpers::JsonBody body;
            try
            {
                const std::string name = HttpServerHelpers::url_decode(nameRaw);
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
    CROW_ROUTE(app, "/api/records/<string>").methods(crow::HTTPMethod::POST)(
        [&engine](const crow::request& req,  const std::string& nameRaw)
        {
            using namespace std::literals;
            HttpServerHelpers::JsonBody body;
            try
            {
                const std::string name = HttpServerHelpers::url_decode(nameRaw);
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
                    body.add("error"sv, "Request JSON format: root element must be an Object"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                const auto iter = requestDocument.FindMember("value");
                if (iter == requestDocument.MemberEnd())
                {
                    body.add("error"sv, "Request JSON format: root object must have 'value' member"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

                const auto& value = iter->value;
                if (!value.IsString())
                {
                    body.add("error"sv, "Request JSON format: root object's 'value' member must have value of type string"sv);
                    return crow::response(crow::status::BAD_REQUEST, body);
                }

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
