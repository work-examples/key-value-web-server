#pragma once

#include <cstdio>


namespace stdlib_extra
{
    class FileOwner
    {
    public:
        FileOwner(std::FILE* file) : m_file(file)
        {
        }

        void close()
        {
            if (m_file != nullptr)
            {
                std::fclose(m_file);
                m_file = nullptr;
            }
        }

        ~FileOwner()
        {
            close();
        }

    public:
        std::FILE* m_file = nullptr;
    };
}
