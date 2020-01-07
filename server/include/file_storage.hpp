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

	FileStorage(const std::string& dir) :
		dir_(dir)
	{
		if (!fs::exists(dir_))
			fs::create_directory(dir_);
	}

	void put_data(data_map_type&& data)
	{
		key_type key;
		std::string file_name;
		std::tie(key, file_name) = write(std::move(data));

		store_index(key, file_name);
	}

	void get_data(key_type key, std::vector<value_type>& result) const
	{
		auto files = read_index(key);
		for (auto& file_name : files)
		{
			data_map_type m = read(file_name);
			auto range = m.equal_range(key);
			for (auto it = range.first; it != range.second; ++it)
			{
				result.push_back(it->second);
			}
		}
	}

private:

	void store_index(key_type key, const std::string& file_name)
	{
		std::unique_lock<std::mutex> lock(index_mtx);
		index.emplace(key, file_name);
	}

	std::vector<std::string> read_index(key_type key) const
	{
		std::vector<std::string> files;

		std::unique_lock<std::mutex> lock(index_mtx);

		auto it_start = index.lower_bound(key);
		auto it_end = index.upper_bound(key);

		if (it_start != index.begin() && (it_start == index.end() || it_start->first > key))
			--it_start;

		for (auto it = it_start; it != it_end; ++it)
		{
			files.push_back(it->second);
		}

		return files;
	}

	std::pair<key_type, std::string> write(data_map_type&& data) const
	{
		if (data.empty())
			throw std::runtime_error("Attempt to store empry data set");

		auto start_point = data.begin()->first;
		auto end_point = data.rbegin()->first;

		std::string file_name = start_point + "_" + end_point + ".dat";
		fs::path file_path = dir_ / file_name;
		std::ofstream output(file_path, std::ios::binary | std::ios::trunc | std::ios::out);
		if (!output.is_open())
			throw std::runtime_error("Couldn't open file " + file_path.string() + " for writing");

		for (auto const& kv : data)
		{
			size_t key_size = kv.first.size();
			output.write((char*)&key_size, sizeof(size_t));
			output.write((char*)kv.first.data(), key_size);

			size_t value_size = kv.second.size();
			output.write((char*)&value_size, sizeof(size_t));
			output.write((char*)kv.second.data(), value_size);
		}
		output.close();

		return { start_point, file_path.string() };
	}

	data_map_type read(const std::string& file_name) const
	{
		data_map_type result;

		char size_buff[sizeof(size_t)];

		std::ifstream input(file_name, std::ios::binary | std::ios::in);
		if (!input.is_open())
			throw std::runtime_error("Couldn't open file " + file_name + " for reading");

		input.seekg(0, std::ios::end);
		std::streampos file_size = input.tellg();
		input.seekg(0, std::ios::beg);

		size_t size = 0;
		while (size < file_size)
		{
			input.read(size_buff, sizeof(size_t));
			size_t key_size = *(size_t*)(size_buff);
			size += sizeof(size_t);

			std::string key;
			key.resize(key_size);
			input.read(key.data(), key_size);
			size += key_size;

			input.read(size_buff, sizeof(size_t));
			size_t value_size = *(size_t*)(size_buff);
			size += sizeof(size_t);

			std::string value;
			value.resize(value_size);
			input.read(value.data(), value_size);
			size += value_size;

			result.emplace(key, value);
		}
		input.close();

		return result;
	}

private:
	fs::path dir_;
	std::map<key_type, std::string> index;
	mutable std::mutex index_mtx;
};

#endif //FILE_STORAGE_HPP_
