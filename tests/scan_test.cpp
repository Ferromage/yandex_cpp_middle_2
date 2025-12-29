#include <gtest/gtest.h>
#include <print>

#include "scan.hpp"

TEST(ScanTest, Simple) {
    {
        auto res = stdx::scan<std::string>("number", "{}");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), std::string("number"));
    }
    {
        const std::string input = "I want to sum 42 and 3.14 numbers.";
        const std::string fmt = "I want to sum {} and {%f} numbers.";
        auto res = stdx::scan<int, float>(input, fmt);
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), 42);
        EXPECT_EQ(std::get<1>(res.value().values()), 3.14f);
    }
}

TEST(ScanTest, IntStringFloat) {
    auto res = stdx::scan<int, std::string, double>("42 hello 3.14", "{%d} {%s} {%f}");
    ASSERT_TRUE(res.has_value());
    const auto &[i, s, d] = res.value().values();
    EXPECT_EQ(i, 42);
    EXPECT_EQ(s, "hello");
    EXPECT_DOUBLE_EQ(d, 3.14);
}

TEST(ScanTest, SingleInt) {
    auto res = stdx::scan<int>("-100", "{%d}");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(std::get<0>(res.value().values()), -100);
}

TEST(ScanTest, UnsignedInt) {
    auto res = stdx::scan<unsigned int>("4000", "{%u}");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(std::get<0>(res.value().values()), 4000u);
}

TEST(ScanTest, StringOnly) {
    auto res = stdx::scan<std::string>("world", "{%s}");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(std::get<0>(res.value().values()), "world");
}

TEST(ScanTest, Empty) {
    {
        auto res = stdx::scan<>({}, {});
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
    {
        auto res = stdx::scan<>("something string", {});
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
    {
        auto res = stdx::scan<>({}, "{%d}");
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
    {
        auto res = stdx::scan<int>("", "{%d}");
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
}

TEST(ScanTest, IntegerInt8) {
    {
        auto res = stdx::scan<int8_t>("100", "{}");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), static_cast<int8_t>(100));
    }
    {
        auto res = stdx::scan<int8_t>("-100", "{}");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), static_cast<int8_t>(-100));
    }
    {
        auto res = stdx::scan<int8_t>("127", "{}");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), static_cast<int8_t>(127));
    }
    {
        auto res = stdx::scan<int8_t>("-128", "{}");
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(std::get<0>(res.value().values()), static_cast<int8_t>(-128));
    }
    {
        auto res = stdx::scan<int8_t>("128", "{}");
        ASSERT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
    {
        auto res = stdx::scan<int8_t>("-129", "{}");
        ASSERT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
}

TEST(ScanTest, InvalidInt) {
    auto res = stdx::scan<int>("abc", "{%d}");
    EXPECT_FALSE(res.has_value());
    EXPECT_NE(res.error().message, "");
}

TEST(ScanTest, FormatTypeMismatch) {
    {
        auto res = stdx::scan<int>("hello", "{%s}");
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
    {
        auto res = stdx::scan<std::string>("123", "{%d}");
        EXPECT_FALSE(res.has_value());
        EXPECT_NE(res.error().message, "");
    }
}

TEST(ScanTest, TooFewSpecifiersInFormat) {
    auto res = stdx::scan<int, std::string>("42 hello", "{%d}");
    EXPECT_FALSE(res.has_value());
    EXPECT_NE(res.error().message, "");
}

TEST(ScanTest, TooManySpecifiersInFormat) {
    auto res = stdx::scan<int>("42", "{%d} {%s}");
    EXPECT_FALSE(res.has_value());
    EXPECT_NE(res.error().message, "");
}

TEST(ScanTest, ConstRefTypes) {
    auto res = stdx::scan<const int &, std::string &&>("42 text", "{%d} {%s}");
    ASSERT_TRUE(res.has_value());
    const auto &[i, s] = res.value().values();
    EXPECT_EQ(i, 42);
    EXPECT_EQ(s, "text");
}

TEST(ScanTest, NegativeFloat) {
    auto res = stdx::scan<double>("-3.14", "{%f}");
    ASSERT_TRUE(res.has_value());
    EXPECT_DOUBLE_EQ(std::get<0>(res.value().values()), -3.14);
}