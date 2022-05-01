#pragma once

#include "rapidjson/document.h"

#include <functional>
#include <string>
#include <string_view>


class DataSerializer
{
public:
    using ItemVisitorProc = void(const std::string_view name, const std::string_view value);

    class Document
    {
    public:
        Document();
        void add(const std::string_view name, const std::string_view value);

        const rapidjson::Document& get() const
        {
            return m_document;
        }

    protected:
        rapidjson::Document m_document;
    };

    static bool load(const std::string& filename, const std::function<ItemVisitorProc>& visitor);

    static bool save(const std::string& filename, const Document& document);
};
