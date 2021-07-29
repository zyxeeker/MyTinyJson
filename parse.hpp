//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_PARSE_HPP
#define MYTINYJSON_PARSE_HPP

#include "define.h"
#include <string>

#if 0
static int ParseKey(JSON_CONTENT *src_content, const std::string *dst_str, JSON_TYPE type){
    std::string *tmp = src_content->content;
    int i = 0;
    for (; i<tmp->length(); ++i) {
        if (tmp+i != dst_str+i)
            return JSON_PARSE_INVALID_VALUE;
    }
    tmp+i;
    JSON_KEY *key;
    key->type = type;
    return JSON_PARSE_OK;
}
#endif

static int ParseKey(std::string *src_str, const std::string *dst_str, JSON_TYPE type) {
    std::string::iterator src = src_str->begin();
    std::string::iterator src_end = src_str->end();
    std::string::const_iterator dst = dst_str->begin();
    std::string::const_iterator dst_end = dst_str->end();

    for (; src != src_end; ++dst, ++src) {
        if (dst == dst_end && src != src_end)
            return JSON_PARSE_INVALID_VALUE;
        if (*src != *dst)
            return JSON_PARSE_INVALID_VALUE;
    }
    return JSON_PARSE_OK;
}


#endif //MYTINYJSON_PARSE_HPP
