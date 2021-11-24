//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_PARSE_HPP
#define MYTINYJSON_PARSE_HPP

#include <string>
#include <cstdlib>
#include <cmath>
#include "define.h"

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define Init(v) do{(v)->type = JSON_NULL;}while(0)

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

void SetString(JSON_VALUE *value, const char *s, size_t len);

static int ParseKey(std::string *src_str, const std::string *dst_str, JSON_TYPE type) {
    auto src = src_str->begin();
    auto src_end = src_str->end();
    auto dst = dst_str->begin();
    auto dst_end = dst_str->end();

    for (; src != src_end; ++dst, ++src) {
        if (dst == dst_end && src != src_end)
            return JSON_PARSE_INVALID_VALUE;
        if (*src != *dst)
            return JSON_PARSE_INVALID_VALUE;
    }
    return JSON_PARSE_OK;
}

static double ParseNumber(std::string *src_str, JSON_TYPE type) {
    auto src = src_str->begin();
    auto src_end = src_str->end();

#if 1
    // 负数
    if (*src == '-') ++src;
    else if (!ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
    // 个数
    if (src == src_end) return JSON_PARSE_INVALID_VALUE;
    // 开头为0
    if (*src == '0') ++src;
    else {
        if (!ISDIGIT1TO9(*src)) return JSON_PARSE_INVALID_VALUE;
        for (; src != src_end && ISDIGIT(*src); ++src);
    }
    // 小数点
    if (src != src_end && *src == '.') {
        ++src;
        if (src == src_end || !ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
        for (; src != src_end && ISDIGIT(*src); ++src);
    }
    // 科学计数法
    if (src != src_end) {
        if (*src == 'E' || *src == 'e') {
            ++src;
            if (*src == '-' || *src == '+') ++src;
            if (!ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
            for (++src; src != src_end && ISDIGIT(*src); ++src);
        }
    }
#endif

#if 0
    if (*src == '-') ++src;
    else if (!ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
    if (src == src_end) return JSON_PARSE_INVALID_VALUE;
    if (*src == '0') ++src;
    else{
        if (src == src_end) return JSON_PARSE_INVALID_VALUE;
        for (;src != src_end; ++src){
            if (ISDIGIT1TO9(*src)) continue;
            else break;
        }
    }
    if (*src == '.' && src == src_str->begin()) return JSON_PARSE_INVALID_VALUE;
    else{
        ++src;
        if (src == src_end) return JSON_PARSE_INVALID_VALUE;
        for (;src != src_end; ++src){
            if (ISDIGIT(*src)) continue;
            else break;
        }
    }
    if (src != src_end) {
        if (*src == 'E' || *src == 'e') {
            if (src != src_end) ++src;
            else return JSON_PARSE_INVALID_VALUE;
        }
        else{
            if (src != src_end){
                if (*src == '-' || *src == '+') ++src;
            }
            else return JSON_PARSE_INVALID_VALUE;
            for (;src != src_end; ++src)
                if (ISDIGIT(*src)) continue;
                else break;

        }
    }
#endif
    double num = strtod(src_str->c_str(), nullptr);
    errno = 0;
    if (errno == ERANGE && (num == HUGE_VAL || num == -HUGE_VAL))
        return JSON_PARSE_NUMBER_TOO_BIG;
    return JSON_PARSE_OK;

}

static void Free(JSON_VALUE *value) {
    assert(value != nullptr);
    if (value->type == JSON_STRING)
        delete value->s.s;
    value->type = JSON_NULL;
}

static int ParseString(JSON_CONTENT *content, JSON_VALUE *value) {
    const char *s;
    assert(*content->json == '\"');
    ++content->json;
    s = content->json;
    for (;; ++s) {
        switch (*s) {
            case '\"':SetString(value, content->json, content->size);
                return JSON_PARSE_OK;
            case '\0':return JSON_PARSE_MISS_QUOTATION_MARK;
            default:++content->size;
        }
    }
}

// NUM
static double GetNum() {

}

static void SetNum() {

}

// BOOL
static int GetBool() {

}

static void SetBool() {

}

// STRING
static void SetString(JSON_VALUE *value, const char *s, size_t len) {
    assert(value != nullptr && (s != nullptr || len == 0));
    Free(value);
    // 有脏数据 所以初始化为null
    value->s.s = new char[len + 1]();
    memcpy(value->s.s, s, len);
    value->s.s[len + 1] = '\0';
    value->s.len = len;
    value->type = JSON_STRING;
}

static std::string GetString(JSON_VALUE *value) {
    return value->s.s;
}

static size_t GetStringLength(JSON_VALUE *value) {
    return value->s.len;
}

#endif //MYTINYJSON_PARSE_HPP
