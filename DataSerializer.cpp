#include "DataSerializer.h"

#include "Logger.h"
#include "utils/stdlib.h"

#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include <cstdio>

#ifdef _MSC_VER
#pragma warning( disable : 4996 ) // warning C4996: 'fopen': This function or variable may be unsafe.
#endif


namespace
{
    constexpr size_t StackBufferSize = 32768;
}

DataSerializer::Document::Document()
{
    m_document.SetObject();
}

void DataSerializer::Document::add(const std::string_view name, const std::string_view value)
{
    const rapidjson::Document::StringRefType nameRef(name.data(), static_cast<rapidjson::SizeType>(name.size()));
    const rapidjson::Document::StringRefType valueRef(value.data(), static_cast<rapidjson::SizeType>(value.size()));

    m_document.AddMember(nameRef, valueRef, m_document.GetAllocator());
}

bool DataSerializer::load(const std::string& filename, const std::function<ItemVisitorProc>& visitor)
{
    stdlib_extra::FileOwner file(std::fopen(filename.c_str(), "rb"));
    if (file.m_file == nullptr)
    {
        LOG_ERROR << "Failed opening the file for reading: " << filename << std::endl;
        return false;
    }

    char readBuffer[StackBufferSize];
    rapidjson::FileReadStream stream(file.m_file, readBuffer, sizeof(readBuffer));

    rapidjson::Document doc;
    doc.ParseStream(stream);

    file.close();

    if (doc.HasParseError())
    {
        LOG_ERROR << "File parsing failed: " << filename << std::endl;
        return false;
    }

    if (!doc.IsObject())
    {
        LOG_ERROR << "JSON root element is not an object. Type: " << static_cast<int>(doc.GetType()) << std::endl;
        return false;
    }

    const auto iterEnd = doc.MemberEnd();
    for (auto iter = doc.MemberBegin(); iter != iterEnd; ++iter)
    {
        const auto& name = iter->name;
        const auto& value = iter->value;
        if (!name.IsString())
        {
            LOG_ERROR << "JSON object element name is not a string. Type: " << static_cast<int>(name.GetType()) << std::endl;
            return false;
        }
        if (!value.IsString())
        {
            LOG_ERROR << "JSON object element value is not a string. Type: " << static_cast<int>(name.GetType()) << "; name: " << name.GetString() << std::endl;
            return false;
        }

        const std::string_view nameView = { name.GetString(), name.GetStringLength() };
        const std::string_view valueView = { value.GetString(), value.GetStringLength() };

        visitor(nameView, valueView);
    }

    return true;
}

bool DataSerializer::save(const std::string& filename, const Document& document)
{
    stdlib_extra::FileOwner file(std::fopen(filename.c_str(), "wb"));
    if (file.m_file == nullptr)
    {
        LOG_ERROR << "Failed opening the file for writing: " << filename << std::endl;
        return false;
    }

    char writeBuffer[StackBufferSize];
    rapidjson::FileWriteStream stream(file.m_file, writeBuffer, sizeof(writeBuffer));

    rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);
    document.get().Accept(writer);

    file.close();

    return true;
}
