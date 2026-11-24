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
 *     version 0.1.0 - introduced ss_free, ss_count, ss_count_overlap, ss_concat and ss_replace,
 *                     fixed cstrbuf_from_ss and split functions
 *     version 0.0.0 - first version
 */

#ifndef STRING_SLICE_H_
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

/* frees underlying char *, use only for string slices, which has its data allocated on heap */
static inline void ss_free(String_Slice *s)
{
    /* since data is allocated in heap its actually char *, so we can safely cast it */
    free((void *)s->data);
}

/* returns null terminated string allocated on heap */
static inline char *cstr_from_ss(String_Slice *s)
{
    char *cstr = calloc(s->len + 1, sizeof(char));
    memcpy(cstr, s->data, s->len);
    return cstr;
}

/* copies string to char buffer */
static inline void cstrbuf_from_ss(String_Slice *s, char *buf, size_t bufsize)
{
    if (bufsize == 0) return;
    size_t n = bufsize - 1 > s->len ? s->len : bufsize - 1;
    memcpy(buf, s->data, n);
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

/* split functions return array of string slices allocated on heap (n is a len of array) */
String_Slice *ss_split_by_delim(String_Slice *s, char delim, size_t *n);
String_Slice *ss_split_by_kind(String_Slice *s, int(*kind)(int), size_t *n);

size_t ss_count(String_Slice *s, String_Slice *sub);
/* counts all substrings including overlapping ones */
size_t ss_count_overlap(String_Slice *s, String_Slice *sub);
String_Slice ss_concat(String_Slice *s1, String_Slice *s2);
String_Slice ss_replace(String_Slice *s, String_Slice *old, String_Slice *new);

#endif /* STRING_SLICE_H_ */

#ifdef STRING_SLICE_IMPLEMENTATION

String_Slice *ss_split_by_delim(String_Slice *s, char delim, size_t *n)
{
    size_t len = 0;
    size_t cap = 128;
    String_Slice *sss = malloc(cap * sizeof(String_Slice));
    const char *beg = s->data;

    for (size_t i = 0; i < s->len; ++i) {
        if (s->data[i] == delim && s->data + i - beg == 0) {
            beg += 1;
        } else if (s->data[i] == delim) {
            if (len == cap) {
                cap *= 2;
                sss = realloc(sss, cap * sizeof(String_Slice));
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
    if (s->data + s->len > beg) {
        sss[len++] = (String_Slice) {
            .data = beg,
            .len = s->data + s->len - beg,
        };
    }
    *n = len;
    return sss;
}

String_Slice *ss_split_by_kind(String_Slice *s, int(*kind)(int), size_t *n)
{
    size_t len = 0;
    size_t cap = 128;
    String_Slice *sss = malloc(cap * sizeof(String_Slice));
    const char *beg = s->data;

    for (size_t i = 0; i < s->len; ++i) {
        if (kind(s->data[i]) && s->data + i - beg == 0) {
            beg += 1;
        } else if (kind(s->data[i])) {
            if (len == cap) {
                cap *= 2;
                sss = realloc(sss, cap * sizeof(String_Slice));
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
    if (s->data + s->len > beg) {
        sss[len++] = (String_Slice) {
            .data = beg,
            .len = s->data + s->len - beg,
        };
    }
    *n = len;
    return sss;
}

size_t ss_count(String_Slice *s, String_Slice *sub)
{
    if (sub->len == 0 || sub->len > s->len) return 0;

    size_t count = 0;
    for (int i = 0; i + sub->len <= s->len; ++i) {
        if (memcmp(s->data + i, sub->data, sub->len) == 0) {
            count += 1;
            i += sub->len - 1;
            continue;
        }
    }
    return count;
}

/* counts all substrings including overlapping ones */
size_t ss_count_overlap(String_Slice *s, String_Slice *sub)
{
    if (sub->len == 0 || sub->len > s->len) return 0;

    size_t count = 0;
    for (int i = 0; i + sub->len <= s->len; ++i) {
        if (memcmp(s->data + i, sub->data, sub->len) == 0) {
            count += 1;
        }
    }
    return count;
}

String_Slice ss_concat(String_Slice *s1, String_Slice *s2)
{
    String_Slice s;
    s.len = s1->len + s2->len;
    char *data = calloc(s.len, sizeof(char));
    memcpy(data, s1->data, s1->len);
    memcpy(data + s1->len, s2->data, s2->len);
    s.data = data;
    return s;
}

String_Slice ss_replace(String_Slice *s, String_Slice *old, String_Slice *new)
{
    String_Slice ss = {0};

    ss.len = s->len + ss_count(s, old) * (new->len - old->len);
    char *str = calloc(ss.len, sizeof(char));

    for (int i = 0, j = 0; j < s->len; ++i, ++j) {
        str[i] = s->data[j];
        if (j + old->len <= s->len && memcmp(s->data + j, old->data, old->len) == 0) {
            memcpy(str + i, new->data, new->len);
            i += new->len - 1;
            j += old->len - 1;
            continue;
        }
    }

    ss.data = str;
    return ss;
}

#endif /* STRING_SLICE_IMPLEMENTATION */
