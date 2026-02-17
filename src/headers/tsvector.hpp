#pragma once

#include <vector>
#include <shared_mutex>
#include <memory>

template <typename T>
class TSVector
{
public:
    TSVector(size_t size)
    {
        reserve(size);
    }
    
    void reserve(size_t size)
    {
        std::unique_lock lock(m_mutex);
        m_vector.reserve(size);
    }

    void resize(size_t size)
    {
        std::unique_lock lock(m_mutex);
        m_vector.resize(size);
    }

    void push_back(const T& value)
    {
        std::unique_lock lock(m_mutex);
        m_vector.push_back(value);
    }

    void emplace_back(T&& value)
    {
        std::unique_lock lock(m_mutex);
        m_vector.emplace_back(std::forward<T>(value));
    }

    std::shared_ptr<std::vector<T>> copyAsShared()
    {
        std::shared_lock lock(m_mutex);
        return std::make_shared<std::vector<T>>(m_vector);
    }

private:
    std::vector<T> m_vector;
    std::shared_mutex m_mutex;
};