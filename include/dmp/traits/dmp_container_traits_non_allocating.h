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


#ifndef DIFF_MATCH_PATCH_CONTAINERTRAITS_CONSTEXPR_H
#define DIFF_MATCH_PATCH_CONTAINERTRAITS_CONSTEXPR_H


#include "dmp/utils/dmp_smallmap.h"
#include "dmp/utils/dmp_smallvector.h"
#include "dmp/utils/dmp_traits.h"


#ifndef NON_ALLOCATING_MAX_DIFFS_LIST_SIZE
#    define NON_ALLOCATING_MAX_DIFFS_LIST_SIZE 64
#endif
#ifndef NON_ALLOCATING_MAX_PATCHES_LIST_SIZE
#    define NON_ALLOCATING_MAX_PATCHES_LIST_SIZE 8
#endif
#ifndef NON_ALLOCATING_MAX_PATCHES_RESULT_LIST_SIZE
#    define NON_ALLOCATING_MAX_PATCHES_RESULT_LIST_SIZE 8
#endif
#ifndef TEMP_NON_ALLOCATING_MAX_BISECT_LIST_SIZE
#    define TEMP_NON_ALLOCATING_MAX_BISECT_LIST_SIZE 2048
#endif
#ifndef TEMP_NON_ALLOCATING_MAX_EQUALITIES_LIST_SIZE
#    define TEMP_NON_ALLOCATING_MAX_EQUALITIES_LIST_SIZE 1024
#endif
#ifndef NON_ALLOCATING_MAX_STRINGPOOL_LIST_SIZE
#    define NON_ALLOCATING_MAX_STRINGPOOL_LIST_SIZE 384
#endif
#ifndef TEMP_NON_ALLOCATING_MAX_LINE_ENCODING_LIST_SIZE
#    define TEMP_NON_ALLOCATING_MAX_LINE_ENCODING_LIST_SIZE 512
#endif
#ifndef TEMP_NON_ALLOCATING_MAX_LINE_ENCODED_STRING_LIST_SIZE
#    define TEMP_NON_ALLOCATING_MAX_LINE_ENCODED_STRING_LIST_SIZE 2048
#endif
#ifndef TEMP_NON_ALLOCATING_MAX_MATCH_LIST_SIZE
#    define TEMP_NON_ALLOCATING_MAX_MATCH_LIST_SIZE 2048
#endif


namespace dmp {
namespace traits {

struct NonAllocatingContainers {};


template <>
struct container_traits<NonAllocatingContainers> {

    template <typename key, typename value>
    struct encoding_hash : public utils::small_map<key, value, 2048> {
        inline constexpr value& insertAndReturnValue(const key& k, value v) noexcept { return this->emplace(k, v).first->second; }
    };

    template <typename key, typename value>
    struct alphabet_hash : public utils::small_map<key, value, 256> {
        inline constexpr value& insertAndReturnValue(const key& k, value v) noexcept { return this->emplace(k, v).first->second; }

        inline constexpr value valueOr(const key& k, value def) const noexcept {
            if (auto it(this->find(k)); it != this->end()) {
                return it->second;
            }

            return def;
        }
    };


    template <typename type>
    using diffs_list = utils::small_vector<type, NON_ALLOCATING_MAX_DIFFS_LIST_SIZE>;

    template <typename type>
    using patches_list = utils::small_vector<type, NON_ALLOCATING_MAX_PATCHES_LIST_SIZE>;

    template <typename type>
    using bisect_list = utils::small_vector<type, TEMP_NON_ALLOCATING_MAX_BISECT_LIST_SIZE>;

    template <typename type>
    using equalities_list = utils::small_vector<type, TEMP_NON_ALLOCATING_MAX_EQUALITIES_LIST_SIZE>;

    template <typename type>
    using string_pool_list = utils::small_vector<type, NON_ALLOCATING_MAX_STRINGPOOL_LIST_SIZE>;

    template <typename type>
    using encoding_list = utils::small_vector<type, TEMP_NON_ALLOCATING_MAX_LINE_ENCODING_LIST_SIZE>;

    template <typename type>
    using encoded_string_list = utils::small_vector<type, TEMP_NON_ALLOCATING_MAX_LINE_ENCODED_STRING_LIST_SIZE>;

    template <typename type>
    using match_temp_list = utils::small_vector<type, TEMP_NON_ALLOCATING_MAX_MATCH_LIST_SIZE>;

    template <typename type>
    using patch_result_list = utils::small_vector<type, NON_ALLOCATING_MAX_PATCHES_RESULT_LIST_SIZE>;
};


}  // namespace traits

using nonallocating_container_traits = traits::container_traits<traits::NonAllocatingContainers>;

}  // namespace dmp

#endif
