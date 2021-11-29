//
// Created by zyx on 2021/7/29.
//

#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "parse.hpp"

TEST(Value_NULL, test) {
    JSON_CONTENT c;
    JSON_VALUE v;
    c.size = c.top = 0;
    const char *dst = "null";
    c.json = "null";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&c, &v, dst, JSON_NULL));
}

TEST(Value_TRUE, test) {
    JSON_CONTENT c;
    JSON_VALUE v;
    c.size = c.top = 0;
    const char *dst = "true";
    c.json = "true";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&c, &v, dst, JSON_TRUE));
}

TEST(Value_FALSE, test) {
    JSON_CONTENT c;
    JSON_VALUE v;
    c.size = c.top = 0;
    const char *dst = "false";
    c.json = "false";
    EXPECT_EQ(JSON_PARSE_OK, ParseKey(&c, &v, dst, JSON_FALSE));
}

TEST(Value_NUM, test) {
    JSON_CONTENT c;
    JSON_VALUE v;
    c.size = c.top = 0;
    c.json = "1";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v)); /* minimum denormal */
    c.json = "4.9406564584124654e-324";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));
    c.json = "-4.9406564584124654e-324";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));  /* Max subnormal double */
    c.json = "2.2250738585072009e-308";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));
    c.json = "-2.2250738585072009e-308";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));  /* Min normal positive double */
    c.json = "2.2250738585072014e-308";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));
    c.json = "-2.2250738585072014e-308";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));  /* Max double */
    c.json = "1.7976931348623157e+308";
    EXPECT_EQ(JSON_PARSE_OK, ParseNumber(&c, &v));
}

TEST(Value_NUM, ERROR_TEST) {
    JSON_CONTENT c;
    JSON_VALUE v;
    c.size = c.top = 0;
    c.json = "nul";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "?";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "+0";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "+1";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = ".123";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "1.";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "INF";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "inf";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "NAN";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "nan";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
    c.json = "E";
    EXPECT_EQ(JSON_PARSE_INVALID_VALUE, ParseNumber(&c, &v));
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
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
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

TEST(ARRAY_TEST, test) {
    JSON_VALUE v;
    JSON_CONTENT c;
    c.json = "[\"asas\",[1,2]]";
//    c.json = "[[1,2]]";
    c.size = c.top = 0;
    EXPECT_EQ(JSON_PARSE_OK, ParseArray(&c, &v));
}

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}