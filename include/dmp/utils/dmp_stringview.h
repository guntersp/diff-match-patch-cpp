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
 * rebased on the current C# version and added constexpr-ness by guntersp0@gmail.com (Gunter Spöcker):
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


#ifndef DIFF_MATCH_PATCH_STRINGVIEW_H
#define DIFF_MATCH_PATCH_STRINGVIEW_H


#include "dmp/utils/dmp_utils.h"

namespace dmp {
namespace utils {

template <typename char_t>
struct minimal_string_view {
    static constexpr const size_t npos = static_cast<size_t>(-1);

    const char_t* ptr = nullptr;
    size_t        len = 0;

    inline constexpr minimal_string_view() noexcept {}

    inline constexpr minimal_string_view(const char_t* _ptr) noexcept
        : ptr(_ptr)
        , len(stringLength(ptr)) {}

    inline constexpr minimal_string_view(const char_t* _ptr, size_t _len) noexcept
        : ptr(_ptr)
        , len(_len) {}


    constexpr minimal_string_view(const minimal_string_view&) noexcept = default;
    constexpr minimal_string_view(minimal_string_view&&) noexcept      = default;

    constexpr minimal_string_view& operator=(const minimal_string_view&) noexcept = default;
    constexpr minimal_string_view& operator=(minimal_string_view&&) noexcept = default;


    inline constexpr size_t length() const noexcept { return len; }

    inline constexpr const char_t* data() const noexcept { return ptr; }


    inline constexpr bool operator==(char_t o) const noexcept {
        if (len != 1) {
            return false;
        }
        return *ptr == o;
    }
    inline constexpr bool operator==(const char_t* o) const noexcept {
        if (!o) {
            return false;
        }

        auto c1(ptr);
        auto l(len);
        while (l > 0 && *o != static_cast<char_t>('\0')) {
            if (*c1++ != *o++) {
                return false;
            }
            l--;
        }

        return *o == static_cast<char_t>('\0');
    }
    inline constexpr bool operator==(const minimal_string_view& o) const noexcept {
        if (len != o.len) {
            return false;
        }

        auto c1(ptr + len);
        auto c2(o.ptr + o.len);
        while (c1 != ptr) {
            if (*--c1 != *--c2) {
                return false;
            }
        }

        return true;
    }


    inline constexpr bool operator!=(char_t o) const noexcept { return !operator==(o); }
    inline constexpr bool operator!=(const char_t* o) const noexcept { return !operator==(o); }
    inline constexpr bool operator!=(const minimal_string_view& o) const noexcept { return !operator==(o); }


    inline constexpr bool operator<(const minimal_string_view& o) const noexcept {

        size_t ol(o.len);
        size_t l(len <= ol ? len : ol);

        auto c1(ptr);
        auto e(c1 + l);
        auto c2(o.ptr);
        while (c1 < e) {
            if (*c1 < *c2) {
                return true;
            } else if (*c1 > *c2) {
                return false;
            }

            ++c1;
            ++c2;
        }

        return len < ol;
    }

    inline constexpr minimal_string_view substr(size_t pos) const noexcept {
        if (pos >= len) {
            return {};
        }
        return minimal_string_view { ptr + pos, len - pos };
    }

    inline constexpr minimal_string_view substr(size_t pos, size_t l) const noexcept {
        if (pos >= len) {
            return {};
        }
        if (pos + l > len) {
            l = len - pos;
        }
        return minimal_string_view { ptr + pos, l };
    }

    inline constexpr size_t find(char_t needle, size_t start) const noexcept {
        for (size_t i = start; i < len; i++) {
            if (ptr[i] == needle) {
                return i;
            }
        }
        return npos;
    }

    inline constexpr size_t find(minimal_string_view needle, size_t start) const noexcept {
        auto nl(needle.len);

        if (!ptr || len <= 0 || !needle.ptr || nl > len) {
            return npos;
        }

        auto l(len - nl + 1);

        size_t j = 0;
        for (size_t i = start; i < l; i++) {
            for (j = 0; j < nl; j++) {
                if (ptr[i + j] != needle.ptr[j]) {
                    break;
                }
            }
            if (j == nl) {
                return i;
            }
        }
        return npos;
    }

    inline constexpr size_t rfind(char_t needle, size_t start) const noexcept {
        for (size_t i = len; i > start; i--) {
            if (ptr[i - 1] == needle) {
                return i - 1;
            }
        }
        return npos;
    }

    inline constexpr size_t rfind(minimal_string_view needle, size_t last) const noexcept {
        auto nl(needle.len);

        if (!ptr || len <= 0 || !needle.ptr || nl > len) {
            return npos;
        }


        if (last > len - nl) {
            last = len - nl;
        }

        size_t j = 0;
        for (size_t i = last + 1; i > 0; i--) {
            for (j = 0; j < nl; j++) {

                if (ptr[i - 1 + j] != needle.ptr[j]) {
                    j = npos;
                    break;
                }
            }

            if (j != npos) {
                return i - 1;
            }
        }

        return npos;
    }

    inline constexpr size_t find(char_t needle) const noexcept { return find(needle, 0); }

    inline constexpr size_t find(minimal_string_view needle) const noexcept { return find(needle, 0); }

    inline constexpr size_t rfind(char_t needle) const noexcept { return rfind(needle, len - 1); }

    inline constexpr size_t rfind(minimal_string_view needle) const noexcept { return rfind(needle, len - 1); }


    inline constexpr auto begin() const noexcept { return ptr; }
    inline constexpr auto end() const noexcept { return ptr + len; }
};


}  // namespace utils
}  // namespace dmp

#endif
