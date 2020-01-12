/*
 * Diff Match and Patch
 * Copyright 2020 The diff-match-patch Authors.
 * https://github.com/google/diff-match-patch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * Functions for diff, match and patch.
 * Computes the difference between two texts to create a patch.
 * Applies the patch onto another text, allowing for errors.
 *
 * @author fraser@google.com (Neil Fraser)
 *
 * STL-only port by snhere@gmail.com (Sergey Nozhenko)
 * and some tweaks for std::string by leutloff@sundancer.oche.de (Christian Leutloff)
 * rebased on the current C# version and added constexpr-ness by ...@... (Gunter Spöcker):
 *
 * Here is a trivial sample program:

#include "diff_match_patch.h"
#include <string>
using namespace std;
int main(int argc, char **argv) {
    diff_match_patch<all_traits<std_wstring_traits, chrono_clock_traits, std_container_traits>> dmp;

    wstring str1 = L"First string in diff";
    wstring str2 = L"Second string in diff";

    wstring strPatch = dmp.patch_toText(dmp.patch_make(str1, str2));
    auto    out(dmp.patch_apply(dmp.patch_fromText(strPatch), str1));
    wstring strResult(out.text2);

    // here, strResult will equal str2 above.
    std::wcout << strResult << "\n";
    return 0;
}
*/


#ifndef DIFF_MATCH_PATCH_UTILS_H
#define DIFF_MATCH_PATCH_UTILS_H


#include <assert.h>
#include <iterator>
#include <limits>
#include <stdint.h>
#include <utility>

namespace std {}

#ifndef DONT_INCLUDE_STD_STRINGVIEW
#    include <string_view>
#endif

namespace dmp {
namespace utils {

template <typename Type>
inline static constexpr Type(min)(Type v1, Type v2) noexcept {
    return v1 < v2 ? v1 : v2;
}

template <typename Type>
inline static constexpr Type(max)(Type v1, Type v2) noexcept {
    return v1 > v2 ? v1 : v2;
}

template <typename Type>
inline static constexpr void swap(Type& v1, Type& v2) noexcept {
    Type t(std::move(v1));
    v1 = std::move(v2);
    v2 = std::move(t);
}

template <typename char_t>
inline static constexpr bool isDigit(char_t chr) noexcept {
    return chr >= static_cast<char_t>('0') && chr <= static_cast<char_t>('9');
}

template <typename char_t>
inline static constexpr bool isAlpha(char_t chr) noexcept {
    return (chr >= static_cast<char_t>('a') && chr <= static_cast<char_t>('z')) ||  //
           (chr >= static_cast<char_t>('A') && chr <= static_cast<char_t>('Z'));
}

template <typename char_t>
inline static constexpr bool isAlphaNum(char_t chr) noexcept {
    return isDigit(chr) || isAlpha(chr);
}

template <typename char_t>
inline static constexpr bool isControl(char_t chr) noexcept {
    switch (chr) {
        case static_cast<char_t>('\r'):
        case static_cast<char_t>('\n'):
            return true;
    }
    return false;
}


template <typename char_t>
static inline constexpr bool isSpace(char_t ch) noexcept {
    switch (ch) {
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
            return true;
    }
    return false;
}

template <typename T, size_t size>
inline static constexpr size_t array_size(T (&)[size]) noexcept {
    return size;
}


template <typename char_t>
inline static constexpr size_t stringLength(const char_t* str) noexcept {
    if (!str) {
        return 0;
    }
    size_t res = 0;
    while (*str != static_cast<char_t>('\0')) {
        res++;
        str++;
    }
    return res;
}


template <typename container>
inline static constexpr auto get_begin(container& c) noexcept {
    using namespace std;
    return begin(c);
}

template <typename container>
inline static constexpr auto get_end(container& c) noexcept {
    using namespace std;
    return end(c);
}


namespace internal {

template <typename T, typename List, typename... Ts>
inline static constexpr void splice_set(List& list, size_t index, const T& value, const Ts&... values) noexcept {
    list[index] = value;

    if constexpr (sizeof...(values) > 0) {
        splice_set<T>(list, index + 1, values...);
    }
}

template <typename T, typename List>
inline static constexpr void splice_set(List& list, size_t index, const T* values, size_t num) noexcept {
    for (size_t i = 0; i < num; i++) {
        list[index + i] = values[i];
    }
}

template <typename T, typename List, typename... Ts>
inline static constexpr void splice_move(List& list, size_t index, T&& value, Ts&&... values) noexcept {
    list[index] = std::move(value);

    if constexpr (sizeof...(values) > 0) {
        splice_move<T>(list, index + 1, std::forward<Ts>(values)...);
    }
}

template <typename T, typename List>
inline static constexpr void splice_move(List& list, size_t index, T* values, size_t num) noexcept {
    for (size_t i = 0; i < num; i++) {
        list[index + i] = std::move(values[i]);
    }
}

template <typename List, typename resizeCallback>
inline static constexpr size_t splice_base(List& list, size_t s, size_t start, size_t remove, size_t insert, resizeCallback resize) noexcept {
    if (start > s) {
        return false;
    }
    if (start + remove > s) {
        remove = s - start;
    }

    if (remove != insert) {

        if (remove > insert) {
            // objects fit into the range


            size_t diff(remove - insert);

            size_t n(s - start - diff);
            for (size_t i = n; i > 0; i--) {
                size_t j(s - i);
                list[j - diff] = list[j];
            }

            resize(list, s - diff);

        } else {
            size_t diff(insert - remove);

            resize(list, s + diff);

            size_t n(s - start - remove);
            for (size_t i = 0; i < n; i++) {
                size_t j(s + diff - 1 - i);
                list[j] = list[j - diff];
            }
        }
    }

    return true;
}

template <typename List>
inline static constexpr bool splice_base(List& list, size_t start, size_t remove, size_t insert) noexcept {
    size_t s(list.size());

    return splice_base(list, s, start, remove, insert, [](List& list, size_t s) { list.resize(s); });
}

}  // namespace internal


template <typename T, typename List, typename... Ts>
inline static constexpr void splice_container(List& list, size_t start, size_t count, const Ts&... values) noexcept {
    if (!internal::splice_base(list, start, count, sizeof...(values))) {
        return;
    }

    if constexpr (sizeof...(values) > 0) {
        internal::splice_set<T>(list, start, values...);
    }
}

template <typename T, typename List, typename... Ts>
inline static constexpr void splice_container(List& list, size_t start, size_t count, T&& value, Ts&&... values) noexcept {
    if (!internal::splice_base(list, start, count, sizeof...(values) + 1)) {
        return;
    }

    internal::splice_move<T>(list, start, std::forward<T>(value), std::forward<Ts>(values)...);
}


template <typename T, size_t size, typename List>
inline static constexpr void splice_container(List& list, size_t start, size_t count, const T (&values)[size]) noexcept {
    if (!internal::splice_base(list, start, count, size)) {
        return;
    }

    internal::splice_set<T>(list, start, values, size);
}

template <typename T, typename List>
inline static constexpr void splice_container(List& list, size_t start, size_t count, const T* values, size_t size) noexcept {
    if (!internal::splice_base(list, start, count, size)) {
        return;
    }

    internal::splice_set<T>(list, start, values, size);
}


template <typename char_t, typename string_t>
inline constexpr static void str_copy(char_t*& str, size_t& len, const string_t& toAppend) noexcept {
    auto l(toAppend.length());
    if (l > len) {
        l   = len;
        len = 0;
    } else {
        len -= l;
    }

    auto d(toAppend.data());
    while (l > 0) {
        l--;
        *str++ = static_cast<char_t>(*d++);
    }
}

template <typename char_t, typename any_char_t>
inline constexpr static void str_copy(char_t*& str, size_t& len, const any_char_t* chars) noexcept {
    auto l(stringLength(chars));
    if (l > len) {
        l   = len;
        len = 0;
    } else {
        len -= l;
    }

    while (l > 0) {
        l--;
        *str++ = static_cast<char_t>(*chars++);
    }
}


template <typename char_t, typename string_t>
inline constexpr static size_t str_insert(char_t* str, size_t size, size_t pos, const string_t& toAppend) noexcept {
    size_t l(toAppend.length());

    if (!internal::splice_base(str, size, pos, 0, l, [&](auto /*list*/, size_t s) { l = s - size; })) {
        return 0;
    }

    auto d(toAppend.data());
    str += pos;
    for (size_t i = l; i > 0; i--) {
        *str++ = static_cast<char_t>(*d++);
    }

    return l;
}


template <typename char_t>
inline constexpr static size_t str_remove(char_t* str, size_t size, size_t pos, size_t removeLength) noexcept {
    if (!internal::splice_base(str, size, pos, removeLength, 0, [&](auto /*list*/, size_t s) { removeLength = size - s; })) {
        return 0;
    }

    return removeLength;
}


namespace internal {
template <typename int_t, typename char_t>
inline static constexpr const char_t* toIntegerInt2(const char_t* start, const char_t* end, int_t& res) {
    res = 0;
    if (start == end) {
        return nullptr;
    }


    uint8_t v {};

    constexpr const int_t absMax((std::numeric_limits<int_t>::max)()); /* absolute max is always smaller than absolute min*/
    constexpr const int_t saveMultiplyMax((absMax - 9) / 10);          // because: (res*10)+decimal

    while (start != end) {
        auto c(*start);

        v = c - '0';

        // not ok but go on
        if (v > 9 /* we leverage the uint8 overflow here */ /*!manz::global::isDigit(c)*/) {
            return start;
        }

        if (res <= saveMultiplyMax) {
            // we are below the critical numerical limit
            res *= 10;
            res += v;

        } else {
            int_t old(res);

            res *= 10;
            res += v;

            // check overflow -> stop directly
            if ((res - v) / 10 != old) {
                res = 0;
                return nullptr;
            }
        }

        start++;
    }

    return end;
}


template <typename int_t, typename char_t>
inline static constexpr const char_t* toIntegerInt(const char_t* start, const char_t* end, int_t& res) {
    res = 0;
    if (start == end) {
        return nullptr;
    }


    if constexpr (!std::numeric_limits<int_t>::is_signed) {
        // negative is not allowed -> stop directly if negative
        if (*start == '-') {
            return nullptr;

        } else if (*start == '+') {
            // ignore
            start++;
        }

        return toIntegerInt2<int_t, char_t>(start, end, res);

    } else {
        if (*start == '-') {
            start++;


            auto p(toIntegerInt2<int_t, char_t>(start, end, res));
            if (!p) {
                return nullptr;
            }

            constexpr const int_t absMax((std::numeric_limits<int_t>::max)()); /* absolute max is always smaller than absolute min*/

            if (res < absMax) {
                res *= -1;

            } else {
                int_t old(res);
                res *= -1;

                // check overflow -> stop directly
                if ((res * -1) != old) {
                    res = 0;
                    return nullptr;
                }
            }

            return p;

        } else if (*start == '+') {
            // ignore
            start++;
        }


        return toIntegerInt2<int_t, char_t>(start, end, res);
    }
}
}  // namespace internal


template <typename Type, typename char_t>
inline constexpr size_t toString(char_t* data, size_t maxLength, Type value) {
    if (maxLength < 1) {
        return 0;
    }


    if (value == 0) {
        *data = static_cast<char_t>('0');
        return 1;
    }


    size_t l = 0;

    if (std::numeric_limits<Type>::is_signed && value < 0) {
        *data = static_cast<char_t>('-');
        l++;

        value *= -1;
    }


    Type v(1);
    while (v <= value) {
        l++;
        v *= 10;
    }

    auto p(l);

    auto put = [&](char_t c) {
        if (p == 0) {
            return false;
        }
        data[--p] = c;
        return true;
    };

    while (value != 0) {
        v = value % 10;

        if (!put('0' + static_cast<char_t>(v))) {
            return {};
        }

        value -= v;
        value /= 10;
    }

    return l;
}


template <typename string_t, typename int_t>
inline static constexpr bool parseInt(const string_t& s, int_t& v) noexcept {
    auto        d(s.data());
    const auto* end(d + s.length());

    return internal::toIntegerInt<int_t>(d, end, v) == end;
}

}  // namespace utils
}  // namespace dmp

#endif
