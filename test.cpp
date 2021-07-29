//
// Created by zyx on 2021/7/29.
//

#include <iostream>
#include "parse.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(Value_NULL, test) {
    std::string dst = "null";
    std::string src_ = "null x";
    std::string src = "null";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&src, &dst, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseKey(&src_, &dst, JSON_NULL));
}

TEST(Value_TRUE, test) {
    std::string dst = "true";
    std::string src_ = "true x";
    std::string src = "true";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&src, &dst, JSON_TRUE));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseKey(&src_, &dst, JSON_TRUE));
}

TEST(Value_FALSE, test) {
    std::string dst = "false";
    std::string src_ = "false x";
    std::string src = "false";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&src, &dst, JSON_FALSE));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseKey(&src_, &dst, JSON_FALSE));
}

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}