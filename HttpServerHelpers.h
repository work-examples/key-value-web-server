#include "crow/logging.h"
#include "crow/returnable.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <string_view>


namespace HttpServerHelpers
{
    std::string url_decode(const std::string& value);

    class LogHandler : public crow::ILogHandler
    {
    public:
        virtual void log(std::string message, crow::LogLevel level) override;
    };

    class JsonBody : public crow::returnable
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
