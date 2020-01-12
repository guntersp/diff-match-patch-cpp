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


#ifndef DIFF_MATCH_PATCH_ENCODING_H
#define DIFF_MATCH_PATCH_ENCODING_H


#include "utils/dmp_unicode.h"
#include "utils/dmp_utils.h"

namespace dmp {
namespace utils {


template <typename type, typename Stream>
inline constexpr Stream& writeToStream(Stream& s, const type& t) noexcept {
    s << t;
    return s;
}


namespace internal {

template <typename char_t>
inline static constexpr bool hex_digit_value(char_t c, unsigned& hex) noexcept {
    switch (c) {
        case static_cast<char_t>('0'):
            hex = 0;
            return true;
        case static_cast<char_t>('1'):
            hex = 1;
            return true;
        case static_cast<char_t>('2'):
            hex = 2;
            return true;
        case static_cast<char_t>('3'):
            hex = 3;
            return true;
        case static_cast<char_t>('4'):
            hex = 4;
            return true;
        case static_cast<char_t>('5'):
            hex = 5;
            return true;
        case static_cast<char_t>('6'):
            hex = 6;
            return true;
        case static_cast<char_t>('7'):
            hex = 7;
            return true;
        case static_cast<char_t>('8'):
            hex = 8;
            return true;
        case static_cast<char_t>('9'):
            hex = 9;
            return true;
        case static_cast<char_t>('A'):
        case static_cast<char_t>('a'):
            hex = 0xA;
            return true;
        case static_cast<char_t>('B'):
        case static_cast<char_t>('b'):
            hex = 0xB;
            return true;
        case static_cast<char_t>('C'):
        case static_cast<char_t>('c'):
            hex = 0xC;
            return true;
        case static_cast<char_t>('D'):
        case static_cast<char_t>('d'):
            hex = 0xD;
            return true;
        case static_cast<char_t>('E'):
        case static_cast<char_t>('e'):
            hex = 0xE;
            return true;
        case static_cast<char_t>('F'):
        case static_cast<char_t>('f'):
            hex = 0xF;
            return true;
    }
    // throw string_t(string_t(traits::cs(L"Invalid character: ")) + c);
    return false;
}

template <typename char_t, const char safe_chars[], size_t length>
struct generic_safe_char_helper {
    char_t hex_output_chars[length] { 0 };
    size_t safe[0x100] { 0 };

    inline constexpr generic_safe_char_helper() noexcept {
        for (size_t i = 0; i < length; i++) {
            auto c = safe_chars[i];

            safe[c] = i + 1;

            if (c >= 'A' && c <= 'Z') {
                c = 'a' + (c - 'A');
            }
            hex_output_chars[i] = c;
        }
    }
};

static constexpr const char safe_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.~ !*'();/?:@&=+$,#";

template <typename char_t>
using safe_char_helper = generic_safe_char_helper<char_t, safe_chars, utils::array_size(safe_chars)>;

}  // namespace internal


template <typename char_traits>
inline constexpr static bool percent_decode(typename char_traits::char_t* str, size_t& size) noexcept {
    using char_t = typename char_traits::char_t;

    auto s2 = str;
    auto s3 = s2;
    auto s4 = s2;

    unsigned h1 {};
    unsigned h2 {};

    for (auto s1 = str, end = s1 + size; s1 != end; ++s1, ++s2) {
        if (*s1 != char_traits::cast('%')) {
            *s2 = *s1;
        } else {
            auto d1 = *++s1;
            if (!internal::hex_digit_value(d1, h1) || !internal::hex_digit_value(*++s1, h2)) {
                return false;
            }
            *s2 = char_traits::cast((h1 << 4) + h2);
        }
    }
    // Decode UTF-8 string in-place
    unsigned u = 0;
    while (s3 != s2) {
        s3 = utf32_from_utf8<char_t, unsigned>::to_utf32(s3, s2, u);
        s4 = char_traits::utf32::from_utf32(u, s4);
    }
    if (s4 != str + size) {
        size = s4 - str;
    }

    return true;
}


/**
 * Encodes a string with URI-style % escaping.
 * Compatible with JavaScript's encodeURI function.
 */
template <typename char_traits, typename Stream, typename string_view_t>
inline static constexpr Stream& encodeURI(Stream& s, const string_view_t& str) noexcept {
    using char_t = typename char_traits::char_t;
    using utf32  = typename char_traits::utf32;

    constexpr internal::safe_char_helper<char_t> safe_helper;

    using namespace dmp::utils;

    typename utf32::utf32_t u {};
    bool                    needsEscaping = false;

    auto i(str.begin());
    auto end(str.end());
    for (auto it(str.begin()); it != end;) {
        it = utf32::to_utf32(it, end, u);

        if (u >= 0x10000 ||  //
            u >= 0x800 ||    //
            u >= 0x80 ||     //
            !safe_helper.safe[static_cast<unsigned char>(u)]) {
            needsEscaping = true;
            break;
        }
    }


    if (!needsEscaping) {
        writeToStream(s, str);

    } else {
        // Encode as UTF-8, then escape unsafe characters
        unsigned char utf8[4] {};
        for (auto it(str.begin()); it != end;) {
            it = utf32::to_utf32(it, end, u);

            unsigned char* pt = utf8;

            pt = utf32_from_utf8<char_t, typename utf32::utf32_t>::from_utf32(u, pt);


            for (const unsigned char* p = utf8; p < pt; ++p) {
                if (safe_helper.safe[*p]) {
                    writeToStream(s, char_traits::cast(internal::safe_chars[safe_helper.safe[*p] - 1]));
                } else {
                    writeToStream(s, char_traits::cast('%'));
                    writeToStream(s, char_traits::cast(safe_helper.hex_output_chars[(*p & 0xF0) >> 4]));
                    writeToStream(s, char_traits::cast(safe_helper.hex_output_chars[*p & 0xF]));
                }
            }
        }
    }

    return s;
}


template <typename char_traits, typename Stream, typename string_view_t>
inline static constexpr Stream& writeHtml(Stream& s, const string_view_t& str) noexcept {
    using char_t = typename char_traits::char_t;

    for (auto c : str) {
        switch (c) {
            case static_cast<char_t>('&'):
                char_traits::write(s, "&amp;");
                break;

            case static_cast<char_t>('<'):
                char_traits::write(s, "&lt;");
                break;

            case static_cast<char_t>('>'):
                char_traits::write(s, "&gt;");
                break;

            case static_cast<char_t>('\n'):
                char_traits::write(s, "&para;<br>");
                break;

            default:
                char_traits::write(s, c);
                break;
        }
    }

    return s;
}

}  // namespace utils
}  // namespace dmp

#endif
