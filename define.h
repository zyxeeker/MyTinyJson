//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_DEFINE_H
#define MYTINYJSON_DEFINE_H

#define LOG(x) std::cout<<x<<std::endl;

enum JSON_TYPE {
    JSON_NULL = 0,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY
};

enum PARSE_STATE {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_ROOT_NOT_SINGULAR,
    JSON_PARSE_NUMBER_TOO_BIG,
    JSON_PARSE_MISS_QUOTATION_MARK,
    JSON_PARSE_INVALID_STRING_ESCAPE,//5
    JSON_PARSE_INVALID_STRING_CHAR,
    JSON_PARSE_INVALID_UNICODE_HEX,//7
    JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

struct JSON_VALUE {
    union {
        struct { JSON_VALUE *v; size_t len; } a;
        struct { char *s; size_t len; } s;
        double num;
    };
    JSON_TYPE type;
};

struct JSON_CONTENT {
    const char *json;
    char *stack;
    size_t size, top;
};

#endif //MYTINYJSON_DEFINE_H
