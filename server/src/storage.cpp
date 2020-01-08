#include "storage.hpp"

bool Storage::commit(key_type key, int version)
{
	bool is_committed;
	value_type value;
	{
		std::unique_lock<std::mutex> lock(data_mtx);

		auto it = data.find(key);
		if (it == data.end())
			return false;

		is_committed = it->second.commit(version, value);
	}
	if (is_committed)
		block_queue->enqueue(key, value);

	return is_committed;
}

int Storage::put_data(key_type key, value_type&& value)
{
	std::unique_lock<std::mutex> lock(data_mtx);

	VersionList& ls = data[key];
	return ls.add_version(std::move(value));
}

std::vector<value_type> Storage::get_data(key_type key) const
{
	std::vector<value_type> result;
	bool found = false;
	{
		std::unique_lock<std::mutex> lock(data_mtx);
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

bool Storage::lookup(key_type key, std::vector<value_type>& result) const
{
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