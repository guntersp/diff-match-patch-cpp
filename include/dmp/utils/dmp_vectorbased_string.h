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


#ifndef DIFF_MATCH_PATCH_VECTORBASED_STRING_H
#define DIFF_MATCH_PATCH_VECTORBASED_STRING_H


#include "dmp/utils/dmp_utils.h"

namespace dmp {
namespace utils {


template <typename char_t, template <typename> typename list, template <typename> typename string_view>
struct vectorbased_string_t : public list<char_t> {
    using parent = list<char_t>;
    using parent::parent;

    using string_view_t = string_view<char_t>;

    constexpr vectorbased_string_t(vectorbased_string_t&&) noexcept = default;
    constexpr vectorbased_string_t& operator=(vectorbased_string_t&&) noexcept = default;

    inline constexpr vectorbased_string_t(const char_t* s, size_t len) noexcept { assign(s, len); }
    inline constexpr vectorbased_string_t(const string_view_t& s) noexcept { assign(s.data(), s.length()); }
    inline constexpr vectorbased_string_t(const typename string_view_t::string_t& s) noexcept { assign(s.data(), s.length()); }

    inline constexpr operator string_view_t() const noexcept { return string_view_t { this->data(), this->size() }.s; }

    inline constexpr void clear() noexcept { return parent::clear(); }

    inline constexpr size_t length() const noexcept { return parent::size(); }

    inline constexpr bool operator==(const vectorbased_string_t& o) const noexcept {
        return string_view_t { this->data(), this->length() } == string_view_t { o.data(), o.length() };
    }

    inline constexpr bool operator==(const string_view_t& o) const noexcept {
        return string_view_t { this->data(), this->length() } == string_view_t { o.data(), o.length() };
    }

    inline constexpr bool operator==(const typename string_view_t::string_t& o) const noexcept {
        return string_view_t { this->data(), this->length() } == o;
    }

    inline constexpr vectorbased_string_t& operator=(const typename string_view_t::string_t& s) noexcept { return assign(s.data(), s.length()); }

    inline constexpr vectorbased_string_t& operator=(const string_view_t& s) noexcept { return assign(s.data(), s.length()); }

    inline constexpr vectorbased_string_t& operator=(char_t c) noexcept {
        parent::resize(1);
        this->operator[](0) = c;
        return *this;
    }

    inline constexpr vectorbased_string_t& operator=(const vectorbased_string_t& s) noexcept {
        if (this != &s) {
            assign(s.data(), s.length());
        }
        return *this;
    }

    inline constexpr vectorbased_string_t& assign(const char_t* s, size_t len) noexcept {
        parent::resize(len);

        for (size_t i = 0; i < len; i++) {
            this->operator[](i) = s[i];
        }

        return *this;
    }

    inline constexpr vectorbased_string_t& append(const char_t* s, size_t len) noexcept {
        parent::reserve(this->size() + len);

        for (size_t i = 0; i < len; i++) {
            parent::push_back(s[i]);
        }

        return *this;
    }

    inline constexpr vectorbased_string_t& append(const char_t* s) noexcept { return append(s, stringLength(s)); }


    inline constexpr vectorbased_string_t& operator+=(const typename string_view_t::string_t& s) noexcept { return append(s.data(), s.length()); }

    inline constexpr vectorbased_string_t& operator+=(const string_view_t& s) noexcept { return append(s.data(), s.length()); }

    inline constexpr vectorbased_string_t& operator+=(char_t c) noexcept {
        parent::push_back(c);
        return *this;
    }

    inline constexpr vectorbased_string_t& operator+=(const char_t* s) noexcept {
        append(s);
        return *this;
    }

    inline constexpr vectorbased_string_t& operator+=(const vectorbased_string_t& s) noexcept { return append(s.data(), s.length()); }
};


template <typename char_t, template <typename> typename list, template <typename> typename string_view>
inline static constexpr bool operator==(const typename string_view<char_t>::string_t&          o,
                                        const vectorbased_string_t<char_t, list, string_view>& s) noexcept {
    return s == o;
}

template <typename char_t, template <typename> typename list, template <typename> typename string_view>
inline static constexpr bool operator!=(const typename string_view<char_t>::string_t&          o,
                                        const vectorbased_string_t<char_t, list, string_view>& s) noexcept {
    return !(s == o);
}


}  // namespace utils
}  // namespace dmp

#endif
