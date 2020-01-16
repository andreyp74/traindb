#include "gtest/gtest.h"

#include <cstdlib>
#include <iostream>
#include "storage.hpp"

class StorageTest : public testing::Test {
protected:

    void SetUp() override
    {
        std::srand((unsigned int)std::time(nullptr));
    }

    void TearDown() override
    {
    }
    
    Storage storage;
};

TEST_F(StorageTest, empty)
{
	{
        int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	{
        int64_t tp2 = std::chrono::system_clock::now().time_since_epoch().count();
		value_type value;
		bool result = storage.get_data(std::to_string(tp2), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}
}

TEST_F(StorageTest, put_get_commit)
{
	int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v1 = std::rand();
	int ver1 = 0; 
	storage.put_data(std::to_string(tp1), std::to_string(v1), ver1);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	int64_t tp2 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v2 = std::rand();
	int ver2 = 1; 
	storage.put_data(std::to_string(tp2), std::to_string(v2), ver2);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}
	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp2), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	storage.commit(std::to_string(tp1), ver1);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v1));
	}
	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp2), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	storage.commit(std::to_string(tp2), ver2);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v1));
	}
	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp2), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v2));
	}

	int64_t tp3 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v3 = std::rand();
	int ver3 = 2;
	storage.put_data(std::to_string(tp3), std::to_string(v3), ver3);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp3), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	storage.commit(std::to_string(tp3), ver3);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp3), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v3));
	}
}

TEST_F(StorageTest, put_get_single_key_versions)
{
	int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v1 = std::rand();
	int ver1 = 0;
	storage.put_data(std::to_string(tp1), std::to_string(v1), ver1);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_FALSE(result);
		ASSERT_TRUE(value.empty());
	}

	storage.commit(std::to_string(tp1), ver1);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v1));
	}

	auto v2 = std::rand();
	int ver2 = 1; 
	storage.put_data(std::to_string(tp1), std::to_string(v2), ver2);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v1));
	}

	auto v3 = std::rand();
	int ver3 = 2;
	storage.put_data(std::to_string(tp1), std::to_string(v3), ver3);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v1));
	}

	storage.commit(std::to_string(tp1), ver2);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v2));
	}

	auto v4 = std::rand();
	int ver4 = 3; 
	storage.put_data(std::to_string(tp1), std::to_string(v4), ver4);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v2));
	}

	//ver3 stays uncommitted
	storage.commit(std::to_string(tp1), ver4);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v4));
	}

	//after commit of ver3 actual version is still ver4
	storage.commit(std::to_string(tp1), ver3);

	{
		value_type value;
		bool result = storage.get_data(std::to_string(tp1), value);
		ASSERT_TRUE(result);
		EXPECT_EQ(value, std::to_string(v4));
	}
	
}