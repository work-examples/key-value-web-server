#include "DataEngine.h"


std::optional<DataEngine::ExtendedValue> DataEngine::get(const std::string_view name) const
{
    const std::shared_lock lock(m_protectData); // read-only lock

    const auto iter = m_data.find(name);
    if (iter == m_data.cend())
    {
        // I don't think we need to increment m_reads here
        return {};
    }

    // Potentially we can get small inconsistency while copying different statistics values. It is acceptable.
    const auto reads = iter->second.m_reads.fetch_add(1, std::memory_order_acq_rel);
    const auto writes = iter->second.m_writes.load(std::memory_order_acquire);

    m_reads.fetch_add(1, std::memory_order_acq_rel);

    return { std::move(ExtendedValue{iter->second.m_value, {reads, writes}}) };
}

void DataEngine::initial_set(const std::string_view name, const std::string_view value)
{
    const std::shared_lock lock(m_protectData); // read-only lock
    const auto iter = m_data.find(name);
    if (iter == m_data.cend())
    {
        ExtendedValueInternal extended{ std::string(value), 0, 0 };
        m_data.emplace(name, std::move(extended));
    }
    else
    {
        iter->second.m_value = value;
    }
}

void DataEngine::set(const std::string_view name, const std::string_view value)
{
    const std::lock_guard lock(m_protectData); // write lock

    const auto iter = m_data.find(name);
    if (iter == m_data.cend())
    {
        ExtendedValueInternal extended{ std::string(value), 0, 1 };
        m_data.emplace(name, std::move(extended));
    }
    else
    {
        iter->second.m_writes.fetch_add(1, std::memory_order_acq_rel);
        iter->second.m_value = value;
    }

    m_writes.fetch_add(1, std::memory_order_acq_rel);
}

void DataEngine::enumerate(const std::function<EnumerateVisitorProc>& visitor) const
{
    const std::shared_lock lock(m_protectData); // read-only lock

    for (const auto& name_value : m_data)
    {
        visitor(name_value.first, name_value.second.m_value);
    }
}

DataEngine::AccessStatistics DataEngine::get_global_statistics() const
{
    // No need in full consistency here
    return { m_reads.load(std::memory_order_acquire), m_writes.load(std::memory_order_acquire) };
}
