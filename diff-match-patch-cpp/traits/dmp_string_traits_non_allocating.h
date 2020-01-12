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


#ifndef DIFF_MATCH_PATCH_STRINGTRAITS_NON_ALLOCATING_H
#define DIFF_MATCH_PATCH_STRINGTRAITS_NON_ALLOCATING_H


#include "traits/dmp_default_char_traits.h"
#include "utils/dmp_fixedsize_stringpool.h"
#include "utils/dmp_smallvector.h"
#include "utils/dmp_stringwrapper.h"
#include "utils/dmp_stringwrapper_utils.h"
#include "utils/dmp_traits.h"
#include "utils/dmp_vectorbased_string.h"


#ifndef DONT_INCLUDE_STD_STRINGVIEW
#    include <string_view>
#else
#    include "utils/dmp_stringview.h"
#endif


#ifndef NON_ALLOCATING_MAX_STRING_SIZE
#    define NON_ALLOCATING_MAX_STRING_SIZE 1024
#endif

#ifndef NON_ALLOCATING_MAX_LONG_STRING_SIZE
#    define NON_ALLOCATING_MAX_LONG_STRING_SIZE 4096
#endif


namespace dmp {
namespace traits {

#ifndef DONT_INCLUDE_STD_STRINGVIEW
template <typename char_t>
using non_allocating_base_string_view_t = std::basic_string_view<char_t>;
#else
template <typename char_t>
using non_allocating_base_string_view_t = utils::minimal_string_view<char_t>;
#endif

template <typename char_t>
using non_allocating_string_view_t = utils::string_wrapper<non_allocating_base_string_view_t<char_t>, char_t>;


template <typename char_t, template <typename> typename string_list>
using non_allocating_owning_string_t = utils::vectorbased_string_t<char_t, string_list, non_allocating_string_view_t>;


template <typename type>
using non_allocating_string_container = utils::small_vector<type, NON_ALLOCATING_MAX_STRING_SIZE>;

template <typename type>
using non_allocating_long_string_container = utils::small_vector<type, NON_ALLOCATING_MAX_LONG_STRING_SIZE>;


template <typename char_t, template <typename> typename string_list>
struct non_allocating_stringstream_t : private dmp::utils::vectorbased_string_t<char_t, string_list, non_allocating_string_view_t> {
    using parent = dmp::utils::vectorbased_string_t<char_t, string_list, non_allocating_string_view_t>;

    constexpr non_allocating_stringstream_t() noexcept = default;

    using string_t = non_allocating_owning_string_t<char_t, string_list>;

    inline constexpr void clear() noexcept { parent::clear(); }

    inline constexpr string_t str() const noexcept { return { this->data(), this->size() }; }

    template <typename IntType>
    inline constexpr non_allocating_stringstream_t& operator<<(IntType v) noexcept {
        using namespace dmp::utils;
        char    buffer[32] {};
        auto    l(toString(buffer, array_size(buffer), v));
        parent::operator+=(non_allocating_base_string_view_t<char_t>(buffer, l));
        return *this;
    }

    inline constexpr non_allocating_stringstream_t& operator<<(char_t s) noexcept {
        parent::operator+=(s);
        return *this;
    }

    inline constexpr non_allocating_stringstream_t& operator<<(const char_t* s) noexcept {
        parent::operator+=(s);
        return *this;
    }

    inline constexpr non_allocating_stringstream_t& operator<<(const non_allocating_base_string_view_t<char_t>& s) noexcept {
        parent::operator+=(s);
        return *this;
    }
};


struct NonAllocatingStringTraits {};

template <>
struct string_traits<NonAllocatingStringTraits> {
    using char_t      = char;
    using char_traits = traits::default_char_traits<char_t>;

    using owning_string_t              = non_allocating_owning_string_t<char_t, non_allocating_string_container>;
    using long_owning_string_t         = non_allocating_owning_string_t<char_t, non_allocating_long_string_container>;
    using string_view_t                = non_allocating_string_view_t<char_t>;
    static constexpr const size_t npos = static_cast<size_t>(-1);

    using stringstream_t = non_allocating_stringstream_t<char_t, non_allocating_string_container>;

    using encoding_char_t = dmp::traits::string_encoding_char_t;

    template <typename string_view_t, typename owning_string_t, template <typename> class list_t>
    using string_pool_t = utils::fixedsize_string_pool<string_view_t, owning_string_t, list_t>;
};


}  // namespace traits

using non_allocating_string_traits = traits::string_traits<traits::NonAllocatingStringTraits>;

}  // namespace dmp

#endif
