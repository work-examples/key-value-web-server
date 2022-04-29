#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4267 ) // warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable : 4244 ) // warning C4244: '=': conversion from '__int64' to 'unsigned long', possible loss of data
#define _WIN32_WINNT    0x0A00 // Windows 10
#endif

#include "crow.h"

#ifdef _MSC_VER
#pragma warning( pop )
#endif

int main(void)
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello world";
        });

    app.bindaddr("127.0.0.1").port(8000).multithreaded().run();
    return 0;
}
