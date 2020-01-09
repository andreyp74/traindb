#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "block_queue.hpp"
#include "file_storage.hpp"

class BlockQueueTest : public testing::Test {
public:
	const size_t block_size = 8;

    BlockQueueTest()
    {
        file_storage = std::make_shared<FileStorage>("./.local");
        block_queue = std::make_unique<BlockQueue>(file_storage, block_size);
    }

    void SetUp() override
	{
		std::filesystem::create_directory("./.local");
		std::srand((unsigned int)std::time(nullptr));
	}

	void TearDown() override
	{
	}

	std::shared_ptr<FileStorage> file_storage;
    std::unique_ptr<BlockQueue> block_queue;
};

TEST_F(BlockQueueTest, enqueue_lookup)
{
    int64_t tp1 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v1 = std::rand();
	int64_t tp2 = std::chrono::system_clock::now().time_since_epoch().count();
	auto v2 = std::rand();

    {
        std::vector<std::string> vec;
        block_queue->lookup(std::to_string(tp1), vec);
        ASSERT_TRUE(vec.empty());
    }

    {
        block_queue->enqueue(std::to_string(tp1), std::to_string(v1));

        std::vector<std::string> vec;
        block_queue->lookup(std::to_string(tp1), vec);
        ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v1));
    }

    {
        block_queue->enqueue(std::to_string(tp2), std::to_string(v2));

        std::vector<std::string> vec;
        block_queue->lookup(std::to_string(tp2), vec);
        ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], std::to_string(v2));
    }

    {
        block_queue->enqueue(std::to_string(tp1), std::to_string(v2));

        std::vector<std::string> vec;
        block_queue->lookup(std::to_string(tp1), vec);

        ASSERT_EQ(vec.size(), 2U);
		EXPECT_EQ(vec[0], std::to_string(v1));
        EXPECT_EQ(vec[1], std::to_string(v2));
    }
}

TEST_F(BlockQueueTest, more_than_block_size_elems)
{
	std::map<std::string, std::string> input;
	for (size_t i = 0; i < block_size + 1; ++i)
	{
		int64_t key = std::chrono::system_clock::now().time_since_epoch().count();
		auto val = std::rand();

		input.emplace(std::to_string(key), std::to_string(val));
	}

	for (auto& kv : input)
	{
		block_queue->enqueue(kv.first, kv.second);
	}

	for (auto& kv : input)
	{
		std::vector<std::string> vec;
		block_queue->lookup(kv.first, vec);
		ASSERT_EQ(vec.size(), 1U);
		EXPECT_EQ(vec[0], kv.second);
	}
}