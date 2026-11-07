/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2026 Pavel Loginov                                               */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

/*
 * CHANGELOG:
 *
 *     version 0.0.0 - first version
 */

#Ifndef STRING_SLICE_H_
#define STRING_SLICE_H_

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SS_FMT "%.*s"
#define SS_ARG(s) (int)(s.len), (s.data)

typedef struct {
    const char *data;
    size_t len;
} String_Slice;

static inline String_Slice ss(const char *cstr)
{
    return (String_Slice) {
        .data = cstr,
        .len = strlen(cstr),
    };
}

/* returns null terminated string allocated in heap */
static inline char *cstr_from_ss(String_Slice *ss)
{
    char *cstr = calloc(ss->len + 1, sizeof(char));
    memcpy(cstr, ss->data, ss->len);
    return cstr;
}

/* copies string to char buffer */
static inline void cstrbuf_from_ss(String_Slice *ss, char *buf, size_t bufsize)
{
    size_t n = bufsize - 1 > ss->len ? ss->len : bufsize - 1;
    memcpy(buf, ss->data, n);
    buf[n] = '\0';
}

static inline void ss_trim_one_left(String_Slice *s)
{
    if (s->len > 0) {
        s->len -= 1;
        s->data = s->data + 1;
    }
}

static inline void ss_trim_one_right(String_Slice *s)
{
    if (s->len > 0) s->len -= 1;
}

static inline void ss_trim_n_left(String_Slice *s, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        ss_trim_one_left(s);
    }
}

static inline void ss_trim_n_right(String_Slice *s, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        ss_trim_one_right(s);
    }
}

static inline void ss_trim_left(String_Slice *s)
{
    while (s->len && isspace(s->data[0]))
        ss_trim_one_left(s);
}

static inline void ss_trim_right(String_Slice *s)
{
    while (s->len && isspace(s->data[s->len - 1]))
        ss_trim_one_right(s);
}

static inline void ss_trim(String_Slice *s)
{
    ss_trim_left(s);
    ss_trim_right(s);
}

/* split functions return array of string slices allocated in heap (n is a len of array) */
String_Slice *ss_split_by_delim(String_Slice *s, char delim, size_t *n);
String_Slice *ss_split_by_kind(String_Slice *s, int(*kind)(int), size_t *n);

#endif // STRING_SLICE_H_

#ifdef STRING_SLICE_IMPLEMENTATION

String_Slice *ss_split_by_delim(String_Slice *s, char delim, size_t *n)
{
    String_Slice *sss = malloc(128 * sizeof(String_Slice));
    size_t len = 0;
    const char *beg = s->data;

    for (size_t i = 0; i < s->len; ++i) {
        if (s->data[i] == delim) {
            if (len == 128) {
                len *= 2;
                sss = realloc(sss, len * sizeof(String_Slice));
            }

            sss[len] = (String_Slice) {
                .data = beg,
                .len = (&s->data[i] - beg),
            };
            len += 1;
            beg = &s->data[i];

            if (*beg == delim && i < s->len + 1) beg += 1;
        }
    }
    *n = len;
    return sss;
}

String_Slice *ss_split_by_kind(String_Slice *s, int(*kind)(int), size_t *n)
{
    String_Slice *sss = malloc(128 * sizeof(String_Slice));
    size_t len = 0;
    const char *beg = s->data;

    for (size_t i = 0; i < s->len; ++i) {
        if (kind(s->data[i])) {
            if (len == 128) {
                len *= 2;
                sss = realloc(sss, len * sizeof(String_Slice));
            }

            sss[len] = (String_Slice) {
                .data = beg,
                .len = (&s->data[i] - beg),
            };
            len += 1;
            beg = &s->data[i];

            if (kind(*beg) && i < s->len + 1) beg += 1;
        }
    }
    *n = len;
    return sss;
}

#endif // STRING_SLICE_IMPLEMENTATION
