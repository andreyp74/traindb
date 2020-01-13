#include "storage.hpp"

bool Storage::commit(const key_type& key, ver_type version)
{
	bool is_committed;
	value_type value;
	{
		std::unique_lock<std::mutex> lock(data_mtx);

		auto it = data.find(key);
		if (it == data.end())
			return false;

		is_committed = it->second.commit(value, version);
	}
	if (is_committed)
		block_queue->enqueue(key, value);

	return is_committed;
}

void Storage::put_data(const key_type& key, const value_type& value, ver_type version)
{
	std::unique_lock<std::mutex> lock(data_mtx);

	VersionList& ls = data[key];
	ls.add_version(value, version);
}

std::vector<value_type> Storage::get_data(const key_type& key) const
{
	std::vector<value_type> result;
	bool found = false;
	{
		found = lookup(key, result);
	}

	if (!found)
	{
		found = block_queue->lookup(key, result);
	}

	if (!found)
	{
		file_storage->get_data(key, result);
	}

	return result;
}

bool Storage::lookup(const key_type& key, std::vector<value_type>& result) const
{
	std::unique_lock<std::mutex> lock(data_mtx);

	auto it = data.find(key);
	if (it != data.end())
	{
		value_type value;
		if (it->second.get_last(value))
			result.push_back(std::move(value));
		return true;
	}
	return false;
}