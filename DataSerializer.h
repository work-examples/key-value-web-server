#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>


class DataSerializerDocument;


class DataSerializer
{
public:
    using ItemVisitorProc = void(const std::string_view name, const std::string_view value);

    class Document
    {
    public:
        Document();
        ~Document();

        void add(const std::string_view name, const std::string_view value);

        const DataSerializerDocument& get_internal_document() const;

    protected:
        std::unique_ptr<DataSerializerDocument> m_ptrDocument;
    };

    static bool load(const std::string& filename, const std::function<ItemVisitorProc>& visitor);

    static bool save(const std::string& filename, const Document& document);
};
