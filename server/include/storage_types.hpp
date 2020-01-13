#ifndef STORAGE_TYPES_HPP_
#define STORAGE_TYPES_HPP_

#include <string>
#include <map>

using key_type = std::string;
using value_type = std::string;
using data_map_type = std::multimap<key_type, value_type>;

using ver_type = int64_t;

class VersionList;
using data_map_ver_type = std::map<key_type, VersionList>;

#endif //STORAGE_TYPES_HPP_