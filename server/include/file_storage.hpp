#ifndef FILE_STORAGE_HPP_
#define FILE_STORAGE_HPP_

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <shared_mutex>
#include <filesystem>

#include "storage_types.hpp"

namespace fs = std::filesystem;

class FileStorage
{
public:

	FileStorage(const std::string& dir);

	void put_data(data_map_type&& data);
	void get_data(const key_type& key, std::vector<value_type>& result) const;

private:
	FileStorage(FileStorage&) = delete;
	FileStorage& operator=(FileStorage&) = delete;

	void store_index(const key_type& key, const std::string& file_name);
	std::vector<std::string> read_index(const key_type& key) const;

	std::pair<key_type, std::string> write(data_map_type&& data) const;
	data_map_type read(const std::string& file_name) const;

private:
	fs::path dir_;
	std::map<key_type, std::string> index;
	mutable std::mutex index_mtx;
};

#endif //FILE_STORAGE_HPP_
