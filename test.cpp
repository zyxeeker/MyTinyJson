//
// Created by zyx on 2021/7/29.
//

#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "parse.hpp"

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

TEST(Value_NUM, test) {
    std::string t = "1";
    std::string src1 = "4.9406564584124654e-324";
    std::string src2 = "-4.9406564584124654e-324";
    std::string src3 = "2.2250738585072009e-308";
    std::string src4 = "-2.2250738585072009e-308";
    std::string src5 = "2.2250738585072014e-308";
    std::string src6 = "-2.2250738585072014e-308";
    std::string src7 = "1.7976931348623157e+308";

    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src1, JSON_NULL)); /* minimum denormal */
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src2, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src3, JSON_NULL));  /* Max subnormal double */
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src4, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src5, JSON_NULL));  /* Min normal positive double */
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src6, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&src7, JSON_NULL));  /* Max double */
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&t, JSON_NULL));
}

TEST(Value_NUM, ERROR_TEST) {
    std::string src1 = "nul";
    std::string src2 = "?";
    std::string src3 = "+0";
    std::string src4 = "+1";
    std::string src5 = ".123";
    std::string src6 = "1.";
    std::string src7 = "INF";
    std::string src8 = "inf";
    std::string src9 = "NAN";
    std::string src0 = "nan";
    std::string src = "E";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src1, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src2, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src3, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src4, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src5, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src6, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src7, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src8, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src9, JSON_NULL));
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&src0, JSON_NULL));

}

void TEST_STRING(char *expect, char *json) {
    JSON_VALUE v;
    JSON_CONTENT s;
    s.json = json;
    s.size = s.top = 0;
    Init(&v);
    EXPECT_EQ(JSON_PARSE_OK, ParseString(&s, &v));
    EXPECT_EQ(expect, GetString(&v));
//    Free(&v);
}

TEST(STRING, test) {
#if 1
    TEST_STRING("H\nWorld", "\"H\\nWorld\"");
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("H\0World", "\"H\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");
#endif
#if 0
    JSON_VALUE v;
    JSON_CONTENT s;
    s.json = "\"sas\"";
    s.size = s.top = 0;
    Init(&v);
    EXPECT_EQ(JSON_PARSE_OK, ParseString(&s, &v));
    EXPECT_EQ("sas", GetString(&v));
#endif
#if 0
    SetString(&v,"Hello",5);
    EXPECT_EQ("Hello", GetString(&v));
    EXPECT_EQ(5, GetStringLength(&v));
#endif
}

void TEST_STRING_ERROR(PARSE_STATE st, char *json) {
    JSON_VALUE v;
    JSON_CONTENT s;
    s.json = json;
    s.size = s.top = 0;
    s.stack = nullptr;
    Init(&v);
    EXPECT_EQ(st, ParseString(&s, &v));
}

TEST(Value_STRING_ERROR, test) {
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");

    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");

    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}