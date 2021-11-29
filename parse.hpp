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
#define STRING_ERROR(ret) do{content->top = head; return ret;}while(0)
#define PUTC(c, ch) do { *(char*)ContentStackPush(c, sizeof(char)) = (ch); } while(0)

static void *ContentStackPush(JSON_CONTENT *content, size_t c) {
    assert(c > 0);
    if (content->top + c >= content->size) {
        if (content->size == 0)
            content->size = JSON_STRING_STACK_LENGTH;
        while (content->top + c >= content->size)
            content->size += content->size >> 1;  /* content->size * 1.5 */
        content->stack = (char *) realloc(content->stack, content->size);
    }
    void *ret;
    ret = content->stack + content->top;
    content->top += c;
    return ret;
}

static char *ContentStackPop(JSON_CONTENT *content, size_t size) {
    assert(content->top >= size);
    return content->stack + (content->top - size);
}

void SetString(JSON_VALUE *value, const char *s, size_t len);

static PARSE_STATE ParseKey(JSON_CONTENT *content, JSON_VALUE *value, const char *dst, JSON_TYPE type) {
    int i;
    for (i = 0; dst[i]; ++i) {
        std::cout << content->json[i] << std::endl;
        if (content->json[i] != dst[i])
            return JSON_PARSE_INVALID_VALUE;
    }
    content->json += i;
    value->type = type;
    return JSON_PARSE_OK;
}

static PARSE_STATE ParseNumber(JSON_CONTENT *content, JSON_VALUE *value) {
    const char *src = content->json;
#if 1
    // 负数
    if (*src == '-') ++src;
    else if (!ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
    // 个数
    if (*src == '\"') return JSON_PARSE_INVALID_VALUE;
    // 开头为0
    if (*src == '0') ++src;
    else {
        if (!ISDIGIT1TO9(*src)) return JSON_PARSE_INVALID_VALUE;
        for (; *src != '\"' && ISDIGIT(*src); ++src);
    }
    // 小数点
    if (*src != '\"' && *src == '.') {
        ++src;
        if (*src == '\"' || !ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
        for (; *src != '\"' && ISDIGIT(*src); ++src);
    }
    // 科学计数法
    if (*src != '\"') {
        if (*src == 'E' || *src == 'e') {
            ++src;
            if (*src == '-' || *src == '+') ++src;
            if (!ISDIGIT(*src)) return JSON_PARSE_INVALID_VALUE;
            for (++src; *src != '\"' && ISDIGIT(*src); ++src);
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
    value->num = strtod(content->json, nullptr);
    errno = 0;
    if (errno == ERANGE && (value->num == HUGE_VAL || value->num == -HUGE_VAL))
        return JSON_PARSE_NUMBER_TOO_BIG;
    value->type = JSON_NUMBER;
    content->json = src;
    return JSON_PARSE_OK;
}

static void Free(JSON_VALUE *value) {
    assert(value != nullptr);
    if (value->type == JSON_STRING)
        delete value->s.s;
    value->type = JSON_NULL;
}

static const char *ParseHex4(const char *p, unsigned *u) {
    *u = 0;
    for (int i = 0; i < 4; ++i, ++p) {
        *u <<= 4;
        if (*p >= '0' && *p <= '9') { *u |= (*p - '0'); }
        else if (*p >= 'A' && *p <= 'F') { *u |= (*p - 'A' + 10); }
        else if (*p >= 'a' && *p <= 'f') { *u |= (*p - 'a' + 10); }
        else return nullptr;
    }
    return p;
}

static void EncodeUtf8(JSON_CONTENT *content, unsigned u) {
    assert(u >= 0x00 && u <= 0x10FFFF);
    if (u >= 0x00 && u <= 0x7F) {
        PUTC(content, u);
    } else if (u >= 0x80 && u <= 0x7FF) {
        PUTC(content, (0xC0 | ((u >> 6) & 0x1F)));
        PUTC(content, (0x80 | (u & 0x3F)));
    } else if (u >= 0x800 && u <= 0xFFFF) {
        PUTC(content, (0xE0 | ((u >> 12) & 0xFF)));
        PUTC(content, (0x80 | ((u >> 6) & 0x3F)));
        PUTC(content, (0x80 | (u & 0x3F)));
    } else if (u >= 0x10000 && u <= 0x10FFFF) {
        PUTC(content, (0xF0 | ((u >> 18) & 0x7)));
        PUTC(content, (0x80 | ((u >> 12) & 0x3F)));
        PUTC(content, (0x80 | ((u >> 6) & 0x3F)));
        PUTC(content, (0x80 | (u & 0x3F)));
    }
}

static PARSE_STATE ParseString(JSON_CONTENT *content, JSON_VALUE *value) {
    const char *s;
    unsigned u;
    char tmp;
    size_t head = content->top;
    assert(*content->json == '\"');
    ++content->json;
    s = content->json;
    content->stack = new char[256]();
    for (;; ++content->size) {
        tmp = *s++;
        switch (tmp) {
            case '\\':
                switch (*s++) {
                    case '\\':PUTC(content, '\\');
                        break;
                    case '\"':PUTC(content, '\"');
                        break;
                    case '/':PUTC(content, '/');
                        break;
                    case 'b':PUTC(content, '\b');
                        break;
                    case 'f':PUTC(content, '\f');
                        break;
                    case 'n':PUTC(content, '\n');
                        break;
                    case 'r':PUTC(content, '\r');
                        break;
                    case 't':PUTC(content, '\t');
                        break;
                    case 'u':
                        if (!(s = ParseHex4(s, &u)))
                            STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            unsigned h = u;
                            if (*s++ != '\\')
                                STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*s++ != 'u')
                                STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(s = ParseHex4(s, &u)))
                                STRING_ERROR(JSON_PARSE_INVALID_UNICODE_HEX);
                            if (u < 0xDC00 || u > 0xDFFF)
                                STRING_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE);
                            u = 0x10000 + (h - 0xD800) * 0x400 + u - 0xDC00;
                        }
                        EncodeUtf8(content, u);
                        break;
                    default:STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\"':SetString(value, ContentStackPop(content, content->top - head), content->size);
                content->json = s;
                return JSON_PARSE_OK;
            case '\0':STRING_ERROR(JSON_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char) tmp < 0x20)
                    STRING_ERROR(JSON_PARSE_INVALID_STRING_CHAR);
                PUTC(content, tmp);
        }
    }
}

// ARRAY
static PARSE_STATE ParseValue(JSON_CONTENT *content, JSON_VALUE *value);

static PARSE_STATE ParseArray(JSON_CONTENT *content, JSON_VALUE *value) {
    size_t size = 0;
    assert(*content->json == '[');
    ++content->json;

    for (;; ++size) {
        JSON_VALUE value1;
        value->a.v = new JSON_VALUE[256]();
        if (*content->json == ',')
            ++content->json;
        if (ParseValue(content, &value1) == JSON_PARSE_OK)
            memcpy(ContentStackPush(content, sizeof(value1)), &value1, sizeof(value1));
        else
            return JSON_PARSE_INVALID_VALUE;
        if (*content->json == ']') {
            ++content->json;
            break;
        }
    }
    return JSON_PARSE_OK;
}

static PARSE_STATE ParseValue(JSON_CONTENT *content, JSON_VALUE *value) {
    switch (*content->json) {
        case 't':return ParseKey(content, value, "true", JSON_TRUE);
        case 'f':return ParseKey(content, value, "false", JSON_FALSE);
        case 'n':return ParseKey(content, value, "null", JSON_NULL);
        case '"':return ParseString(content, value);
        case '[':return ParseArray(content, value);
        case '\0':return JSON_PARSE_EXPECT_VALUE;
        default:return ParseNumber(content, value);
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
