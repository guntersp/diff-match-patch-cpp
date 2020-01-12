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


#ifndef DIFF_MATCH_PATCH_SUBTRAITS_H
#define DIFF_MATCH_PATCH_SUBTRAITS_H


#include "traits/dmp_default_char_traits.h"
#include "utils/dmp_stringpool.h"
#include "utils/dmp_stringwrapper.h"
#include "utils/dmp_traits.h"
#include "utils/dmp_vectorbased_string.h"


#ifndef DONT_INCLUDE_STD_STRINGVIEW
#    include <string_view>
#else
#    include "utils/dmp_stringview.h"
#endif


namespace dmp {

template <class all_traits>
struct diff_match_patch_diff;

namespace traits {


#ifndef DONT_INCLUDE_STD_STRINGVIEW
template <typename encoding_char_t>
using encoded_base_string_view_t = std::basic_string_view<encoding_char_t>;
#else
template <typename encoding_char_t>
using encoded_base_string_view_t = utils::minimal_string_view<encoding_char_t>;
#endif


template <typename char_t>
using encoded_string_view_t = utils::string_wrapper<encoded_base_string_view_t<char_t>, char_t>;


template <typename encoding_char_t, template <typename> typename encoded_string_list>
using encoded_owning_string_t = utils::vectorbased_string_t<encoding_char_t, encoded_string_list, encoded_string_view_t>;


template <typename _encoding_char_t, typename container_traits,
          template <typename, typename, template <typename> class list_t> typename _string_pool_t>
struct encoded_string_traits {
    using encoding_char_t = _encoding_char_t;

    using char_t      = encoding_char_t;
    using char_traits = traits::default_char_traits<char_t>;

    template <typename list_t>
    using encoded_string_list_t = typename container_traits::template encoded_string_list<list_t>;

    using owning_string_t = encoded_owning_string_t<encoding_char_t, encoded_string_list_t>;
    using string_view_t   = encoded_string_view_t<encoding_char_t>;

    static constexpr const size_t npos = static_cast<size_t>(-1);


    template <typename string_view_t, typename owning_string_t, template <typename> class list_t>
    using string_pool_t = _string_pool_t<string_view_t, owning_string_t, list_t>;
};


template <class algorithm_diff,
          bool isSame = std::is_same_v<typename algorithm_diff::string_traits::char_t, typename algorithm_diff::string_traits::encoding_char_t>>
struct diff_encoding_helper {
    static constexpr const bool needsSubDiff = true;

    template <typename string_view_t, typename owning_string_t, template <typename> class list_t>
    using string_pool_t = typename algorithm_diff::string_traits::template string_pool_t<string_view_t, owning_string_t, list_t>;

    using encoding_string_traits
        = encoded_string_traits<typename algorithm_diff::string_traits::encoding_char_t, typename algorithm_diff::container_traits, string_pool_t>;

    using encoding_all_traits
        = dmp::all_traits<encoding_string_traits, typename algorithm_diff::clock_traits, typename algorithm_diff::container_traits>;

    using algorithm_diff_t = dmp::diff_match_patch_diff<encoding_all_traits>;
};

template <class algorithm_diff>
struct diff_encoding_helper<algorithm_diff, true> {
    static constexpr const bool needsSubDiff = false;

    using algorithm_diff_t = algorithm_diff;
};

template <typename encoding_char_t, template <typename> typename encoded_string_list>
inline static constexpr bool operator==(const encoded_string_view_t<encoding_char_t>&                                o,
                                        const traits::encoded_owning_string_t<encoding_char_t, encoded_string_list>& s) noexcept {
    return s == o;
}

template <typename encoding_char_t, template <typename> typename encoded_string_list>
inline static constexpr bool operator!=(const encoded_string_view_t<encoding_char_t>&                                o,
                                        const traits::encoded_owning_string_t<encoding_char_t, encoded_string_list>& s) noexcept {
    return s != o;
}
}  // namespace traits

namespace utils {

template <typename encoding_char_t, template <typename> typename encoded_string_list>
inline constexpr static void str_assign(const traits::encoded_owning_string_t<encoding_char_t, encoded_string_list>& toAssign,
                                        traits::encoded_string_view_t<encoding_char_t>&                              str) noexcept {
    str = traits::encoded_string_view_t<encoding_char_t> { toAssign.data(), toAssign.size() };
}


}  // namespace utils
}  // namespace dmp

#endif
