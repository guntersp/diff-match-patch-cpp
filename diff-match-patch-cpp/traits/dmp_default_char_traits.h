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


#ifndef DIFF_MATCH_PATCH_DEFAULT_CHAR_TRAITS_H
#define DIFF_MATCH_PATCH_DEFAULT_CHAR_TRAITS_H


#include "utils/dmp_unicode.h"
#include "utils/dmp_utils.h"

namespace dmp {
namespace traits {

template <typename char_t, size_t size = sizeof(char_t)>
struct utf32_helper {
    using utf32 = dmp::utils::utf32_from_utf16<char_t>;
};

template <typename char_t>
struct utf32_helper<char_t, 4> {
    using utf32 = dmp::utils::utf32_direct<char_t>;
};

template <typename char_t>
struct utf32_helper<char_t, 1> {
    using utf32 = dmp::utils::utf32_from_utf8<char_t>;
};

template <typename _char_t, typename _utf32 = typename utf32_helper<_char_t>::utf32>
struct default_char_traits {
    using char_t = _char_t;
    using utf32  = _utf32;

    inline static constexpr bool isAlphaNum(char_t c) { return utils::isAlphaNum(c); }
    inline static constexpr bool isDigit(char_t c) { return utils::isDigit(c); }
    inline static constexpr bool isSpace(char_t c) { return utils::isSpace(c); }
    inline static constexpr bool isControl(char_t c) { return utils::isControl(c); }

    template <typename any_char_t>
    inline static constexpr char_t cast(any_char_t c) {
        return static_cast<char_t>(c);
    }

    template <typename string_traits, typename any_char_t>
    inline constexpr static typename string_traits::owning_string_t cast_string(const any_char_t* c) noexcept {
        if (!c) {
            return {};
        }

        using namespace dmp::utils;

        size_t l(stringLength(c));

        typename string_traits::owning_string_t r;
        r.reserve(l);

        while (*c != '\0') {
            str_append(r, cast(*c++));
        }

        return r;
    }

    static constexpr const char_t eol = static_cast<char_t>('\n');
    static constexpr const char_t ret = static_cast<char_t>('\r');
    static constexpr const char_t tab = static_cast<char_t>('\t');

    template <typename Stream>
    inline static constexpr Stream& write(Stream& s, char_t c) noexcept {
        s << c;
        return s;
    }

    template <typename Stream>
    inline static constexpr Stream& write(Stream& s, const char_t* str) noexcept {
        s << str;
        return s;
    }


    template <typename Stream, typename any_char_t>
    inline static constexpr Stream& write(Stream& s, any_char_t c) noexcept {
        s << cast(c);
        return s;
    }

    template <typename Stream, typename any_char_t>
    inline static constexpr Stream& write(Stream& s, const any_char_t* str) noexcept {
        while (str && *str != cast('\0')) {
            s << cast(*str);
            str++;
        }
        return s;
    }
};

}  // namespace traits
}  // namespace dmp

#endif
