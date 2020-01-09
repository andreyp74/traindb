#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include <string>
#include <chrono>
#include <map>
#include <mutex>

#include "version_list.hpp"
#include "file_storage.hpp"
#include "block_queue.hpp"


class Storage
{
public:
	Storage() 
	{
		file_storage = std::make_shared<FileStorage>("./.local/");
		block_queue = std::make_unique<BlockQueue>(file_storage);
		block_queue->start();
	}

	~Storage()
	{
		block_queue->stop();
	}

	bool commit(const key_type& key, int version);
	int put_data(const key_type& key, value_type&& value);

	std::vector<value_type> get_data(const key_type& key) const;

private:
	Storage(Storage&) = delete;
	Storage& operator=(Storage&) = delete;

	bool lookup(const key_type& key, std::vector<value_type>& result) const;
	
private:

	std::shared_ptr<FileStorage> file_storage;
	std::unique_ptr<BlockQueue> block_queue;

	data_map_ver_type data;
	mutable std::mutex data_mtx;
};

#endif //STORAGE_HPP_