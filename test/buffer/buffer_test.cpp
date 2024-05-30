#include "../../code/buffer/buffer.h"
#include <gtest/gtest.h>
#include <string>

// Test fixture for Buffer class
class BufferTest : public ::testing::Test {
protected:
    Buffer buffer;

    BufferTest() : buffer(1024) {}
};

// Test for initial state
TEST_F(BufferTest, InitialState) {
    EXPECT_EQ(buffer.ReadableBytes(), 0);
    EXPECT_EQ(buffer.WritableBytes(), 1024);
    EXPECT_EQ(buffer.PrependableBytes(), 0);
}

// Test for Append and Retrieve functionality
TEST_F(BufferTest, AppendAndRetrieve) {
    std::string data = "Hello, World!";
    buffer.Append(data);

    EXPECT_EQ(buffer.ReadableBytes(), data.size());
    EXPECT_EQ(buffer.WritableBytes(), 1024 - data.size());
    EXPECT_EQ(buffer.PrependableBytes(), 0);

    std::string result = buffer.RetrieveAllToString();
    EXPECT_EQ(result, data);
    EXPECT_EQ(buffer.ReadableBytes(), 0);
    EXPECT_EQ(buffer.WritableBytes(), 1024);
}

// Test for EnsureWritable and MakeSpace functionality
TEST_F(BufferTest, EnsureWritable) {
    std::string data(2048, 'x');
    buffer.Append(data);

    EXPECT_EQ(buffer.ReadableBytes(), 2048);
    EXPECT_EQ(buffer.WritableBytes(), 1024 - 2048 % 1024);
    EXPECT_EQ(buffer.PrependableBytes(), 0);
}

// Test for RetrieveUntil functionality
TEST_F(BufferTest, RetrieveUntil) {
    std::string data = "Hello, World!";
    buffer.Append(data);

    const char* end = buffer.Peek() + 5;
    buffer.RetrieveUntil(end);

    EXPECT_EQ(buffer.ReadableBytes(), data.size() - 5);
    EXPECT_EQ(buffer.WritableBytes(), 1024 - data.size());
    EXPECT_EQ(buffer.PrependableBytes(), 5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
