#pragma once

#include <map>
#include <iterator>
#include <thread>
#include <shared_mutex>
#include <optional>
#include <iostream>

template <typename Key, typename Object>
class ThreadSafeMap
{
public:
	// Standart using type for self-writed containers
	using value_type = std::pair<const Key, Object>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using container_t = std::map<Key, Object>;

	std::optional<value_type> find(Key id) const;
	bool insert(const std::pair<Key, Object>& insertValue);
	size_type erase(Key id);
    bool empty() const;
	container_t copy() const;
	std::shared_ptr<container_t> copyAsShared() const;

protected:
	container_t m_map;
	mutable std::shared_mutex m_mutex;
};

template <typename Key, typename Object>
std::optional<typename ThreadSafeMap<Key, Object>::value_type> ThreadSafeMap<Key, Object>::find(Key id) const
{
	std::shared_lock lock(m_mutex);
	auto container_iterator = m_map.find(id);
	if (container_iterator != std::end(m_map))
	{
		return { *container_iterator };
	}

	return std::nullopt;
}

template <typename Key, typename Object>
bool ThreadSafeMap<Key, Object>::insert(const std::pair<Key, Object>& insertValue)
{
	std::unique_lock lock(m_mutex);
	auto [iterator, isInserted] = m_map.insert(insertValue);
	return isInserted;
}

template <typename Key, typename Object>
typename ThreadSafeMap<Key, Object>::size_type ThreadSafeMap<Key, Object>::erase(Key id)
{
	std::unique_lock lock(m_mutex);
	return m_map.erase(id);
}

template <typename Key, typename Object>
bool ThreadSafeMap<Key, Object>::empty() const
{
	std::shared_lock lock(m_mutex);
    return m_map.empty();
}

template <typename Key, typename Object>
typename ThreadSafeMap<Key, Object>::container_t ThreadSafeMap<Key, Object>::copy() const
{
	std::shared_lock lock(m_mutex);
	return { m_map };
}

template <typename Key, typename Object>
std::shared_ptr<typename ThreadSafeMap<Key, Object>::container_t> ThreadSafeMap<Key, Object>::copyAsShared() const
{
	std::shared_lock lock(m_mutex);
	return std::make_shared<container_t>(m_map);
}