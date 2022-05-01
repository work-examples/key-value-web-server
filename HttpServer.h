#pragma once

#include <cstdint>
#include <memory>
#include <string>


class DataEngine;
class HttpServerApp;


class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    void run(const std::string& host, const std::uint16_t port, const DataEngine& engine);

    void stop_notify();

protected:
    void setup_routing(const DataEngine& engine);

    std::unique_ptr<HttpServerApp> m_ptrApp;
};
