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


#ifndef DIFF_MATCH_PATCH_STRING_POOL_BASE_H
#define DIFF_MATCH_PATCH_STRING_POOL_BASE_H


#include "utils/dmp_encoding.h"
#include "utils/dmp_smallvector.h"
#include "utils/dmp_stringwrapper_utils.h"

namespace dmp {
namespace utils {

template <typename _string_view_t, typename _owning_string_t>
struct string_pool_base {
    using string_view_t   = _string_view_t;
    using owning_string_t = _owning_string_t;

    struct original_texts {
        decltype(static_cast<const string_view_t*>(nullptr)->data()) begin { nullptr };
        decltype(static_cast<const string_view_t*>(nullptr)->data()) end { nullptr };

        inline constexpr original_texts() noexcept {}

        inline constexpr original_texts(const string_view_t* text) noexcept
            : begin(text ? text->data() : nullptr)
            , end(text ? begin + text->length() : nullptr) {

            if (begin == end) {
                begin = nullptr;
                end   = nullptr;
            }
        }
    };

    using creator      = decltype(static_cast<owning_string_t*>(nullptr)->data()) (*)(string_pool_base& pool, size_t length);
    creator createNext = nullptr;

    struct original_texts_data {
        const original_texts* texts = nullptr;
        size_t                num   = 0;

        inline constexpr original_texts_data() noexcept {}

        inline constexpr original_texts_data(const original_texts* texts, size_t num) noexcept
            : texts(texts)
            , num(num) {}
    };
    original_texts_data textsData;
    
    
    template <size_t size>
    inline constexpr string_pool_base(creator createNext, const original_texts (&texts)[size]) noexcept
        : createNext(createNext)
        , textsData { texts, size } {}

    inline constexpr string_pool_base(creator createNext, const original_texts* texts, size_t size) noexcept
        : createNext(createNext)
        , textsData { texts, size } {}

    template <size_t size>
    inline constexpr void setOriginalTexts(const original_texts (&texts)[size]) noexcept {
        textsData.texts = texts;
        textsData.num   = size;
    }

    inline constexpr void setOriginalTexts(const original_texts* texts, size_t size) noexcept {
        textsData.texts = texts;
        textsData.num   = size;
    }

    inline constexpr void resetOriginalTexts() noexcept {
        textsData.texts = nullptr;
        textsData.num   = 0;
    }


    template <typename... strings>
    inline constexpr string_view_t create(size_t totalLength, const strings&... args) noexcept {
        auto* s(createNext(*this, totalLength));
        if (!s) {
            return {};
        }

        using namespace dmp::utils;

        auto l(totalLength);
        auto p(s);

        (str_copy(p, l, args), ...);

        return string_view_t { s, totalLength - l };
    }

    template <typename... strings>
    inline constexpr string_view_t create(const strings&... args) noexcept {
        size_t l = (0 + ... + args.length());

        return create(l, args...);
    }

    template <size_t size, typename char_t>
    inline constexpr string_view_t createFromCCharString(const char_t (&arg)[size]) noexcept {
        return create(size - 1 /* remove trailing '\0' */, arg);
    }

    template <typename... chars>
    inline constexpr string_view_t createFromChars(const chars&... args) noexcept {
        size_t l = (0 + ... + stringLength(args));

        return create(l, args...);
    }


private:
    template <typename Chars, typename... Args>
    inline static constexpr bool checkStringForExpansion(const Chars* /*data*/) noexcept {
        return true;
    }

    template <typename Chars, typename Arg, typename... Args>
    inline static constexpr bool checkStringForExpansion(const Chars* data, const Arg& arg, const Args&... args) noexcept {
        auto l(arg.length());

        if (!(arg.s == string_view_t(data, l))) {
            return false;
        }

        return checkStringForExpansion(data + l, args...);
    }

    template <typename Arg>
    inline constexpr string_view_t appendedInt(size_t /*totalLength*/, const Arg& arg) noexcept {
        // special case
        return static_cast<const string_view_t&>(arg);
    }

    template <typename Arg, typename... Args>
    inline constexpr string_view_t appendedInt(size_t totalLength, const Arg& arg, const Args&... args) noexcept {

        auto l(arg.length());
        if (l == 0) {
            return appendedInt(totalLength, args...);
        }

        auto d(arg.data());

        for (size_t i = 0; i < textsData.num; i++) {
            auto& t(textsData.texts[i]);

            if (t.begin && d >= t.begin && d < t.end) {
                // this string belongs to the original text

                // check if the substrings are all from the original text. If so, just reuse them
                if (d + totalLength <= t.end && checkStringForExpansion(d + l, args...)) {
                    return string_view_t { d, totalLength };
                }
            }
        }

        // fallback, create a new string
        return create(totalLength, arg, args...);
    }

public:
    template <typename... Args>
    inline constexpr string_view_t appended(const string_view_t& first, const Args&... args) noexcept {
        if constexpr (sizeof...(args) == 0) {
            // trivial case
            return first;
        }

        size_t othersLength = (0 + ... + args.length());
        if (othersLength == 0) {
            return first;
        }

        size_t l(first.length());
        if (l == 0) {
            return appendedInt(othersLength, args...);
        }

        return appendedInt(l + othersLength, first, args...);
    }

    template <typename... Args>
    inline constexpr string_view_t& append(string_view_t& first, const Args&... args) noexcept {
        if constexpr (sizeof...(args) <= 0) {
            return first;
        }

        first = appended(first, args...);
        return first;
    }


    template <typename char_traits>
    inline constexpr bool percent_decode(string_view_t& str, bool replacePluses) noexcept {
        using namespace dmp::utils;

        size_t l          = str.length();
        bool   willChange = false;

        if (replacePluses) {
            for (auto& c : str) {
                if (c == char_traits::cast('+')) {
                    l += 2;
                    willChange = true;
                }

                if (c == char_traits::cast('%') || !(c >= 0 && c < 0x80 /* latin1 range*/)) {
                    willChange = true;
                }
            }
        }

        if (!willChange) {
            // nothing to do
            return true;
        }


        auto* s(createNext(*this, l));
        if (!s) {
            return false;
        }

        auto p(s);

        for (auto& c : str) {
            if (replacePluses) {
                if (c == char_traits::cast('+')) {
                    *p++ = char_traits::cast('%');
                    *p++ = char_traits::cast('2');
                    *p++ = char_traits::cast('b');
                    continue;
                }
            }
            *p++ = c;
        }

        if (!utils::percent_decode<char_traits>(s, l)) {
            return false;
        }
        str = string_view_t { s, l };
        return true;
    }
};


}  // namespace utils
}  // namespace dmp

#endif
