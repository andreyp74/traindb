#ifndef VERSION_LIST_HPP_
#define VERSION_LIST_HPP_

#include <list>
#include <string>
#include <map>

using key_type = std::string;
using value_type = std::string;

struct StoredValue
{
	StoredValue(int version, bool clear, value_type value) :
		version(version),
		clear(clear),
		value(value)
	{
	}

	int version;
	bool clear;
	value_type value;
};

class VersionList
{
public:
	bool commit(int version, value_type& value)
	{
		auto it = std::find_if(ver_list.begin(), ver_list.end(), [&](StoredValue& v) { return v.version == version; });
		if (it == ver_list.end())
			return false;

		it->clear = true;
		ver_list.erase(ver_list.begin(), it);
		value = it->value;
		return true;
	}

	int add_version(value_type&& value)
	{
		int version = ver_list.empty() ? 0 : ver_list.back().version + 1;
		ver_list.emplace_back(version, false, value);
		return version;
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

using data_map_ver_type = std::map<key_type, VersionList>;
using data_map_type = std::multimap<key_type, value_type>;

#endif //VERSION_LIST_HPP_