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


#ifndef DIFF_MATCH_PATCH_UNICODE_H
#define DIFF_MATCH_PATCH_UNICODE_H


#include <limits>


namespace dmp {
namespace utils {

template <class char_t, class _utf32_t = unsigned>
struct utf32_direct {
    using utf32_t = _utf32_t;

    static constexpr const utf32_t inputmask = std::numeric_limits<char_t>::max();

    template <class iterator>
    inline static constexpr iterator to_utf32(iterator i, iterator /*end*/, utf32_t& u) noexcept {
        u = *i++;
        u &= inputmask;
        return i;
    }
    template <class iterator>
    inline static constexpr iterator from_utf32(utf32_t u, iterator o) noexcept {
        *o++ = static_cast<char_t>(u);
        return o;
    }
};

template <class char_t, class _utf32_t = unsigned>
struct utf32_from_utf8 {
    using utf32_t = _utf32_t;

    static constexpr const utf32_t inputmask = std::numeric_limits<char_t>::max();

    template <class iterator>
    inline static constexpr iterator to_utf32(iterator i, iterator end, utf32_t& u) noexcept {
        // utf8 -> utf32

        while (i != end) {
            u = static_cast<utf32_t>(*i++);
            // in case char type is signed, this might lead to an unsigned overflow to a really big number
            u &= inputmask;

            if (u < 0x80) {
                return i;

            } else if ((u >> 5) == 6) {
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u = ((u & 0x1F) << 6) + (*i & 0x3F);
                return i;

            } else if ((u >> 4) == 0xE) {
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u = ((u & 0xF) << 12) + ((*i & 0x3F) << 6);
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u += *i & 0x3F;
                return i;

            } else if ((u >> 3) == 0x1E) {
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u = ((u & 7) << 18) + ((*i & 0x3F) << 12);
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u += (*i & 0x3F) << 6;
                if (i == end || (*i & 0xC0) != 0x80) {
                    continue;
                }
                u += *i & 0x3F;
                return i;
            }
        }

        return i;
    }
    template <class iterator>
    inline static constexpr iterator from_utf32(utf32_t u, iterator o) noexcept {
        // utf32 -> utf8
        if (u < 0x80)
            *o++ = static_cast<unsigned char>(u);
        else if (u < 0x800) {
            *o++ = static_cast<unsigned char>((u >> 6) | 0xC0);
            *o++ = static_cast<unsigned char>((u & 0x3F) | 0x80);
        } else if (u < 0x10000) {
            *o++ = static_cast<unsigned char>((u >> 12) | 0xE0);
            *o++ = static_cast<unsigned char>(((u >> 6) & 0x3F) | 0x80);
            *o++ = static_cast<unsigned char>((u & 0x3F) | 0x80);
        } else {
            *o++ = static_cast<unsigned char>((u >> 18) | 0xF0);
            *o++ = static_cast<unsigned char>(((u >> 12) & 0x3F) | 0x80);
            *o++ = static_cast<unsigned char>(((u >> 6) & 0x3F) | 0x80);
            *o++ = static_cast<unsigned char>((u & 0x3F) | 0x80);
        }

        return o;
    }
};

template <class char_t, class _utf32_t = unsigned>
struct utf32_from_utf16 {
    using utf32_t = _utf32_t;

    static constexpr const utf32_t UTF16_SURROGATE_MIN      = 0xd800u;
    static constexpr const utf32_t UTF16_SURROGATE_MAX      = 0xdfffu;
    static constexpr const utf32_t UTF16_HIGH_SURROGATE_MAX = 0xdbffu;
    static constexpr const utf32_t UTF16_LOW_SURROGATE_MIN  = 0xdc00u;
    static constexpr const utf32_t UTF16_SURROGATE_OFFSET   = (UTF16_SURROGATE_MIN << 10) + UTF16_HIGH_SURROGATE_MAX - 0xffffu;

    static constexpr const utf32_t inputmask = (1 << (8 * sizeof(char_t))) - 1;

    template <class iterator>
    inline static constexpr iterator to_utf32(iterator i, iterator end, utf32_t& u) noexcept {
        u = static_cast<utf32_t>(*i++);
        // in case char type is signed, this might lead to an unsigned overflow to a really big number
        u &= inputmask;

        if (UTF16_SURROGATE_MIN <= u && u <= UTF16_HIGH_SURROGATE_MAX && i != end)
            u = (u << 10) + *i++ - UTF16_SURROGATE_OFFSET;  // Assume it is a UTF-16 surrogate pair
        return i;
    }
    template <class iterator>
    inline static constexpr iterator from_utf32(utf32_t u, iterator o) noexcept {
        if (u > 0xffff) {  // Encode code points that do not fit in char_t as UTF-16 surrogate pairs
            *o++ = static_cast<char_t>((u >> 10) + UTF16_SURROGATE_MIN - (0x10000 >> 10));
            *o++ = static_cast<char_t>((u & 0x3ff) + UTF16_LOW_SURROGATE_MIN);
        } else
            *o++ = static_cast<char_t>(u);
        return o;
    }
};


}  // namespace utils
}  // namespace dmp

#endif
