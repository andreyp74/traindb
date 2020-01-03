#include "gtest/gtest.h"

#include <cstdlib>
#include <iostream>
#include <filesystem>
#include "file_storage.hpp"

class FileStorageTest : public testing::Test {
protected:

	FileStorageTest()
		: file_storage("./.local")
	{
	}

	void SetUp() override
	{
		std::filesystem::create_directory("./.local");
		std::srand((unsigned int)std::time(nullptr));
	}

	void TearDown() override
	{
		//std::filesystem::remove("./.local");
	}

	FileStorage file_storage;
};

TEST_F(FileStorageTest, empty)
{
	{
        int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp1), vec);
		ASSERT_TRUE(vec.empty());
	}

	{
        int64_t tp2 = std::chrono::system_clock::now().time_since_epoch().count();
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp2), vec);
		ASSERT_TRUE(vec.empty());
	}
}

TEST_F(FileStorageTest, put_get)
{
	int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v1 = std::rand();
	int64_t tp2 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v2 = std::rand();
	int64_t tp3 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v3 = std::rand();

	data_map_type m1;
	m1.insert({ std::to_string(tp1), std::to_string(v1) });
	m1.insert({ std::to_string(tp2), std::to_string(v2) });
	m1.insert({ std::to_string(tp3), std::to_string(v3) });
	file_storage.put_data(std::move(m1));

	{
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp1), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v1));
    }
    {
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp2), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v2));
    }
	{
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp3), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v3));
    }

	int64_t tp4 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v4 = std::rand();
	int64_t tp5 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v5 = std::rand();
	int64_t tp6 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v6 = std::rand();

	data_map_type m2;
	m2.insert({ std::to_string(tp4), std::to_string(v4) });
	m2.insert({ std::to_string(tp5), std::to_string(v5) });
	m2.insert({ std::to_string(tp6), std::to_string(v6) });
	file_storage.put_data(std::move(m2));

	{
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp4), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v4));
	}
	{
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp5), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v5));
	}
	{
		std::vector<value_type> vec;
		file_storage.get_data(std::to_string(tp6), vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v6));
	}
}