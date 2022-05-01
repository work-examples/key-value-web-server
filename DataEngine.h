#pragma once

#include "utils/stl.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>


class DataEngine
{
public:
    using IntegerCounter = std::uint64_t;

    struct AccessStatistics
    {
        IntegerCounter          m_reads = 0;
        IntegerCounter          m_writes = 0;
    };

    struct ExtendedValue
    {
        std::string             m_value;
        AccessStatistics        m_stats;
    };

public:
    std::optional<ExtendedValue> get(const std::string_view name) const;

    void initial_set(const std::string_view name, const std::string_view value); // no statistics update
    void set(const std::string_view name, const std::string_view value);

    using EnumerateVisitorProc = void(const std::string_view name, const std::string_view value);

    void enumerate(const std::function<EnumerateVisitorProc>& visitor) const;

    AccessStatistics get_global_statistics() const;

protected:
    class AtomicCounter : public std::atomic<IntegerCounter>
    {
    public:
        // Define copy-constructor to allow easier emplacing of such values into unordered_map values
        AtomicCounter(const IntegerCounter value = 0) noexcept
        {
            this->store(value, std::memory_order_release);
        }
        AtomicCounter(const AtomicCounter& obj) noexcept
        {
            const auto value = obj.load(std::memory_order_acquire);
            this->store(value, std::memory_order_release);
        }
    };

    struct ExtendedValueInternal
    {
        std::string             m_value;
        mutable AtomicCounter   m_reads  = ATOMIC_VAR_INIT(0);
        AtomicCounter           m_writes = ATOMIC_VAR_INIT(0);
    };

    using DataCollection = stl_extra::string_unordered_map<ExtendedValueInternal>;
    mutable std::shared_mutex   m_protectData;
    DataCollection              m_data;

    // Global statistics:
    mutable std::atomic<IntegerCounter> m_reads  = ATOMIC_VAR_INIT(0);
    std::atomic<IntegerCounter>         m_writes = ATOMIC_VAR_INIT(0);
};
