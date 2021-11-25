//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_PARSE_HPP
#define MYTINYJSON_PARSE_HPP

#include <string>
#include <cstdlib>
#include <cmath>
#include "define.h"

#define JSON_STRING_STACK_LENGTH 256
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define Init(v) do{(v)->type = JSON_NULL;}while(0)

static void StringStackPush(JSON_CONTENT *content, char c) {
    size_t size = sizeof(char);
    assert(size > 0);
    if (content->top + size >= content->size) {
        if (content->size == 0)
            content->size = JSON_STRING_STACK_LENGTH;
        while (content->top + size >= content->size)
            content->size += content->size >> 1;  /* content->size * 1.5 */
        content->stack = (char *) realloc(content->stack, content->size);
    }
    content->stack[content->top] = c;
    content->top += size;
}

static char *StringStackPop(JSON_CONTENT *content, size_t size) {
    assert(content->top >= size);
    return content->stack + (content->top - size);
}

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
    size_t head = content->top, len;
    assert(*content->json == '\"');
    ++content->json;
    s = content->json;
    content->stack = new char[256]();
    for (;; ++s, ++content->size) {
        switch (*s) {
            case '\\':
                switch (*(s + 1)) {
                    case '\\':StringStackPush(content, '\\');
                        break;
                    case '\"':StringStackPush(content, '\"');
                        break;
                    case '/':StringStackPush(content, '/');
                        break;
                    case 'b':StringStackPush(content, '\b');
                        break;
                    case 'f':StringStackPush(content, '\f');
                        break;
                    case 'n':StringStackPush(content, '\n');
                        break;
                    case 'r':StringStackPush(content, '\r');
                        break;
                    case 't':StringStackPush(content, '\t');
                        break;
                    default:content->top = head;
                        return JSON_PARSE_INVALID_STRING_ESCAPE;
                }
                ++s;
                break;
            case '\"':SetString(value, StringStackPop(content, content->top - head), content->size);
                return JSON_PARSE_OK;
            case '\0':return JSON_PARSE_MISS_QUOTATION_MARK;
            default:
                if ((unsigned char) *s <= 0x20) {
                    content->top = head;
                    return JSON_PARSE_INVALID_STRING_CHAR;
                }
                StringStackPush(content, *s);
        }
    }
}

// NUM
static double GetNum(JSON_VALUE *value) {
    assert(value != nullptr && value->type == JSON_NUMBER);
    return value->num;
}

static void SetNum(JSON_VALUE *value, double num) {
    assert(value != nullptr);
    Free(value);
    value->num = num;
    value->type = JSON_NUMBER;
}

// BOOL
static int GetBool(JSON_VALUE *value) {
    assert(value != nullptr && (value->type == JSON_TRUE || value->type == JSON_FALSE));
    return value->type == JSON_TRUE;
}

static void SetBool(JSON_VALUE *value, int b) {
    assert(value != nullptr);
    Free(value);
    b ? value->type = JSON_TRUE : JSON_FALSE;
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
