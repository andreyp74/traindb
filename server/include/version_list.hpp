#ifndef VERSION_LIST_HPP_
#define VERSION_LIST_HPP_

#include <list>
#include <algorithm>

#include "storage_types.hpp"

struct StoredValue
{
	StoredValue(ver_type version, bool clear, value_type value) :
		version(version),
		clear(clear),
		value(value)
	{
	}

	ver_type version;
	bool clear;
	value_type value;
};

class VersionList
{
public:
	bool commit(value_type& value, ver_type version)
	{
		auto it = std::find_if(ver_list.begin(), ver_list.end(), [&](StoredValue& v) { return v.version == version; });
		if (it == ver_list.end())
			return false;

		it->clear = true;
		ver_list.erase(ver_list.begin(), it);
		value = it->value;
		return true;
	}

	void add_version(const value_type& value, ver_type version)
	{
		ver_list.emplace_back(version, false, value);
	}

	bool get_last(value_type& value) const
	{
		if (ver_list.empty())
			return false;

		const auto it = std::find_if(ver_list.crbegin(), ver_list.crend(), [&](const StoredValue& v) { return v.clear == true; });
		if (it == ver_list.crend())
			return false;

		value = it->value;
		return true;
	}

private:
	std::list<StoredValue> ver_list;
};


#endif //VERSION_LIST_HPP_