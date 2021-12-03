//
// Created by zyx on 2021/7/29.
//

#ifndef MYTINYJSON_PARSE_HPP
#define MYTINYJSON_PARSE_HPP

#include <string>
#include <cstdlib>
#include <cmath>
#include "define.h"

#define JSON_STACK_LENGTH 256
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define Init(v) do{(v)->type = JSON_NULL;}while(0)
#define STRING_ERROR(ret) do{content->top = head; return ret;}while(0)
#define PUTC(c, ch) do { *(char*)ContentStackPush(c, sizeof(char)) = (ch); } while(0)
#define PUTS(c, s, len)     memcpy((char*)ContentStackPush(c, len), s, len)

static const char hexDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static void *ContentStackPush(JSON_CONTENT *content, size_t c) {
    assert(c > 0);
    if (content->top + c >= content->size) {
        if (content->size == 0)
            content->size = JSON_STACK_LENGTH;
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
//    栈顶指针退回
    return content->stack + (content->top -= size);
}

static void ParseWhitespace(JSON_CONTENT *content) {
    const char *p = content->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    content->json = p;
}

void SetString(JSON_VALUE *value, const char *s, size_t len);

static PARSE_STATE ParseKey(JSON_CONTENT *content, JSON_VALUE *value, const char *dst, JSON_TYPE type) {
    int i;
    for (i = 0; dst[i]; ++i) {
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
    switch (value->type) {
        case JSON_STRING:free(value->s.s);
            break;
        case JSON_ARRAY:
//            释放堆栈中的数据
            for (size_t i = 0; i < value->a.len; ++i)
                Free(&value->a.v[i]);
//            释放本身
            Free(value->a.v);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < value->o.len; ++i) {
                free(value->o.m[i].k);
                Free(&value->o.m[i].v);
            }
            free(value->o.m);
            break;
    }
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

static void EncodeUtf8(JSON_CONTENT *content, unsigned u, size_t *len) {
    assert(u >= 0x00 && u <= 0x10FFFF);
    if (u >= 0x00 && u <= 0x7F) {
        PUTC(content, u);
    } else if (u >= 0x80 && u <= 0x7FF) {
        PUTC(content, (0xC0 | ((u >> 6) & 0x1F)));
        PUTC(content, (0x80 | (u & 0x3F)));
        ++*len;
    } else if (u >= 0x800 && u <= 0xFFFF) {
        PUTC(content, (0xE0 | ((u >> 12) & 0xFF)));
        PUTC(content, (0x80 | ((u >> 6) & 0x3F)));
        PUTC(content, (0x80 | (u & 0x3F)));
        *len += 2;
    } else if (u >= 0x10000 && u <= 0x10FFFF) {
        PUTC(content, (0xF0 | ((u >> 18) & 0x7)));
        PUTC(content, (0x80 | ((u >> 12) & 0x3F)));
        PUTC(content, (0x80 | ((u >> 6) & 0x3F)));
        PUTC(content, (0x80 | (u & 0x3F)));
        *len += 3;
    }
}

static PARSE_STATE ParseRawString(JSON_CONTENT *content, char **str, size_t *len) {
    unsigned u;
    char tmp;
    const char *s;
    size_t head = content->top;
    assert(*content->json == '\"');
    ++content->json;
    s = content->json;
    for (;; ++*len) {
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
                        EncodeUtf8(content, u, len);
                        break;
                    default:STRING_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\"':*str = ContentStackPop(content, *len);
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

static PARSE_STATE ParseString(JSON_CONTENT *content, JSON_VALUE *value) {
#if 1
    PARSE_STATE ret;
    char *s;
    size_t len = 0;
    if ((ret = ParseRawString(content, &s, &len)) == JSON_PARSE_OK)
        SetString(value, s, len);
    return ret;
#else
    const char *s;
    unsigned u;
    char tmp;
    size_t head = content->top, len = 0;
    assert(*content->json == '\"');
    ++content->json;
    s = content->json;
    for (;; ++len) {
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
            case '\"':SetString(value, ContentStackPop(content, len), len);
                content->json = s;
                return JSON_PARSE_OK;
            case '\0':STRING_ERROR(JSON_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char) tmp < 0x20)
                    STRING_ERROR(JSON_PARSE_INVALID_STRING_CHAR);
                PUTC(content, tmp);
        }
    }
#endif
}

// ARRAY
static PARSE_STATE ParseValue(JSON_CONTENT *content, JSON_VALUE *value);

static PARSE_STATE ParseArray(JSON_CONTENT *content, JSON_VALUE *value) {
    size_t i, size = 0;
    PARSE_STATE ret;
    assert(*content->json == '[');
    ++content->json;
    ParseWhitespace(content);
    if (*content->json == ']') {
        value->type = JSON_ARRAY;
        value->a.len = 0;
        value->a.v = nullptr;
        ++content->json;
        return JSON_PARSE_OK;
    }
    for (;;) {
        JSON_VALUE value1{};
        if ((ret = ParseValue(content, &value1)) != JSON_PARSE_OK)
            break;
        memcpy(ContentStackPush(content, sizeof(value1)), &value1, sizeof(value1));
        ++size;
        ParseWhitespace(content);
        if (*content->json == ',') {
            ++content->json;
            ParseWhitespace(content);
        } else if (*content->json == ']') {
            ++content->json;
            value->type = JSON_ARRAY;
            value->a.len = size;
            size *= sizeof(JSON_VALUE);
            memcpy(value->a.v = (JSON_VALUE *) malloc(size), ContentStackPop(content, size), size);
            return JSON_PARSE_OK;
        } else
            return JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
    }
//    解析错误时释放前面已解析到堆栈中的数据
    for (i = 0; i < size; ++i)
        Free(reinterpret_cast<JSON_VALUE *>(ContentStackPop(content, size)));
    return ret;
}

static PARSE_STATE ParseObject(JSON_CONTENT *content, JSON_VALUE *value);

static PARSE_STATE ParseValue(JSON_CONTENT *content, JSON_VALUE *value) {
    switch (*content->json) {
        case 't':return ParseKey(content, value, "true", JSON_TRUE);
        case 'f':return ParseKey(content, value, "false", JSON_FALSE);
        case 'n':return ParseKey(content, value, "null", JSON_NULL);
        case '"':return ParseString(content, value);
        case '[':return ParseArray(content, value);
        case '{':return ParseObject(content, value);
        case '\0':return JSON_PARSE_EXPECT_VALUE;
        default:return ParseNumber(content, value);
    }
}

// OBJECT
static PARSE_STATE ParseObject(JSON_CONTENT *content, JSON_VALUE *value) {
    size_t size, i, len;
    i = size = len = 0;
    JSON_MEMBER m{};
    PARSE_STATE ret;
    assert(*content->json == '{');
    ++content->json;
    ParseWhitespace(content);
    if (*content->json == '}') {
        content->json++;
        value->type = JSON_OBJECT;
        value->o.m = nullptr;
        value->o.len = 0;
        return JSON_PARSE_OK;
    }
    m.k = nullptr;
    for (;;) {
        char *tmp = nullptr;
        len = 0;
        m.v.type = JSON_NULL;
        if (*content->json != '"') {
            ret = JSON_PARSE_MISS_KEY;
            break;
        } else if ((ret = ParseRawString(content, &tmp, &len)) != JSON_PARSE_OK)
            break;
        memcpy(m.k = (char *) malloc(len + 1), tmp, len);
        m.k[len] = '\0';
        m.len = len;
        ParseWhitespace(content);
        if (*content->json != ':') {
            ret = JSON_PARSE_MISS_COLON;
            break;
        }
        ++content->json;
        ParseWhitespace(content);
        if ((ret = ParseValue(content, &m.v)) != JSON_PARSE_OK)
            break;
        memcpy(ContentStackPush(content, sizeof(JSON_MEMBER)), &m, sizeof(JSON_MEMBER));
        ++size;
        m.k = nullptr;
        ParseWhitespace(content);
        if (*content->json == ',') {
            ++content->json;
            ParseWhitespace(content);
        } else if (*content->json == '}') {
            size_t s = sizeof(JSON_MEMBER) * size;
            value->type = JSON_OBJECT;
            value->o.len = size;
            content->json++;
            memcpy(value->o.m = (JSON_MEMBER *) malloc(s), ContentStackPop(content, s), s);
            return JSON_PARSE_OK;
        } else {
            ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    free(m.k);
    for (i = 0; i < size; ++i) {
        auto mTmp = reinterpret_cast<JSON_MEMBER *>(ContentStackPop(content, sizeof(JSON_MEMBER)));
        free(mTmp->k);
        Free(&mTmp->v);
    }
    return ret;
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
    value->s.s[len] = '\0';
    value->s.len = len;
    value->type = JSON_STRING;
}

static std::string GetString(JSON_VALUE *value) {
    assert(value != nullptr && value->type == JSON_STRING);
    return value->s.s;
}

static size_t GetStringLength(JSON_VALUE *value) {
    assert(value != nullptr && value->type == JSON_STRING);
    return value->s.len;
}

//OBJECT
size_t GetObjectSize(const JSON_VALUE *value) {
    assert(value != nullptr && value->type == JSON_OBJECT);
    return value->o.len;
}

const char *GetObjectKey(const JSON_VALUE *value, size_t index) {
    assert(value->type == JSON_OBJECT);
    assert(value != nullptr && index < value->o.len);
    return value->o.m[index].k;
}

size_t GetObjectKeyLength(const JSON_VALUE *value, size_t index) {
    assert(value->type == JSON_OBJECT);
    assert(value != nullptr && index < value->o.len);
    return value->o.m[index].len;
}

JSON_VALUE *GetObjectValue(const JSON_VALUE *value, size_t index) {
    assert(value->type == JSON_OBJECT);
    assert(value != nullptr && index < value->o.len);
    return &value->o.m[index].v;
}

JSON_TYPE GetElementType(const JSON_VALUE *v) {
    return v->type;
}

JSON_VALUE *GetArrayElement(const JSON_VALUE *v, size_t index) {
    assert(v != nullptr && v->type == JSON_ARRAY);
    assert(index < v->a.len);
    return &v->a.v[index];
}

size_t GetArraySize(const JSON_VALUE *v) {
    assert(v != nullptr && v->type == JSON_ARRAY);
    return v->a.len;
}

PARSE_STATE ParseJsonString(const char *json, JSON_VALUE *value) {
    assert(value != nullptr);
    PARSE_STATE ret;
    JSON_CONTENT content{};
    content.json = json;
    ParseWhitespace(&content);
    if ((ret = ParseValue(&content, value)) == JSON_PARSE_OK) {
        ParseWhitespace(&content);
        if (*content.json != '\0') {
            value->type = JSON_NULL;
            return JSON_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(content.top == 0);
    free(content.stack);
    return ret;
}

STRINGIFY_STATE JSONStringifyValue(JSON_CONTENT *content, const JSON_VALUE *value);

void JSONStringifyString(JSON_CONTENT *content, const JSON_VALUE *value) {
    assert(value->s.s != nullptr);
    size_t i, size;
    char *head, *p;
    // "\u00xx..."
    p = head = reinterpret_cast<char *>(ContentStackPush(content, size = value->s.len * 6 + 2));
    *p++ = '"';
    for (i = 0; i < value->s.len; i++) {
        auto ch = (unsigned char) value->s.s[i];
        switch (ch) {
            case '\"': *p++ = '\\';
                *p++ = '\"';
                break;
            case '\\': *p++ = '\\';
                *p++ = '\\';
                break;
            case '\b': *p++ = '\\';
                *p++ = 'b';
                break;
            case '\f': *p++ = '\\';
                *p++ = 'f';
                break;
            case '\n': *p++ = '\\';
                *p++ = 'n';
                break;
            case '\r': *p++ = '\\';
                *p++ = 'r';
                break;
            case '\t': *p++ = '\\';
                *p++ = 't';
                break;
            default:
                if (ch < 0x20) {
                    *p++ = '\\';
                    *p++ = 'u';
                    *p++ = '0';
                    *p++ = '0';
                    *p++ = hexDigits[ch >> 4];
                    *p++ = hexDigits[ch & 15];
                } else
                    *p++ = value->s.s[i];
        }
    }
    *p++ = '"';
    content->top -= size - (p - head);

}

STRINGIFY_STATE JSONStringifyValue(JSON_CONTENT *content, const JSON_VALUE *value) {
    size_t i;
    switch (value->type) {
        case JSON_NULL:PUTS(content, "null", 4);
            break;
        case JSON_TRUE:PUTS(content, "true", 4);
            break;
        case JSON_FALSE:PUTS(content, "false", 5);
            break;
        case JSON_NUMBER:
            content->top -= 32 - sprintf(reinterpret_cast<char *>(ContentStackPush(content, 32)), "%.17g", value->num);
            break;
        case JSON_OBJECT:PUTS(content, "{", 1);
            for (i = 0; i < value->o.len; ++i) {
                PUTS(content, "\"", 1);
                PUTS(content, value->o.m[i].k, value->o.m[i].len);
                PUTS(content, "\":", 2);
                JSONStringifyValue(content, &value->o.m[i].v);
                if (i != value->o.len - 1)
                    PUTS(content, ",", 1);
            }
            PUTS(content, "}", 1);
            break;
        case JSON_ARRAY:PUTS(content, "[", 1);
            for (i = 0; i < value->a.len; ++i) {
                JSONStringifyValue(content, &value->a.v[i]);
                if (i != value->a.len - 1)
                    PUTS(content, ",", 1);
            }
            PUTS(content, "]", 1);
            break;
        case JSON_STRING:JSONStringifyString(content, value);
            break;
    }
    return JSON_STRINGIFY_OK;
}

STRINGIFY_STATE JSONStringify(const JSON_VALUE *value, char **json, size_t *l) {
    assert(value != nullptr && json != nullptr);
    JSON_CONTENT content{};
    STRINGIFY_STATE ret;
    content.size = JSON_STACK_LENGTH;
    content.stack = (char *) malloc(JSON_STACK_LENGTH);
    if ((ret = JSONStringifyValue(&content, value)) == JSON_STRINGIFY_OK) {
        PUTC(&content, '\0');
        *json = content.stack;
        *l = content.top;
        return ret;
    }
    free(content.stack);
    return JSON_STRINGIFY_FAIL;
}

#endif //MYTINYJSON_PARSE_HPP
