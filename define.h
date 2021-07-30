//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_DEFINE_H
#define MYTINYJSON_DEFINE_H

enum JSON_TYPE {
    JSON_NULL = 0,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NUMBER
};

enum PARSE_STATE {
    JSON_PARSE_OK = 0,
    JSON_PARSE_EXPECT_VALUE,
    JSON_PARSE_INVALID_VALUE,
    JSON_PARSE_ROOT_NOT_SINGULAR,
    JSON_PARSE_NUMBER_TOO_BIG
};

struct JSON_CONTENT {
    std::string *content;
};

struct JSON_KEY {
    int key;
    JSON_TYPE type;
};

#endif //MYTINYJSON_DEFINE_H
