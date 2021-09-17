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


#ifndef DIFF_MATCH_PATCH_BASE_H
#define DIFF_MATCH_PATCH_BASE_H


#include "dmp/algorithms/dmp_algorithms_patch.h"

namespace dmp {


/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */
template <class all_traits>
struct diff_match_patch_base : public diff_match_patch_diff<all_traits>,
                               public diff_match_patch_match<all_traits>,
                               public diff_match_patch_patch<all_traits> {
    using algorithm_diff  = diff_match_patch_diff<all_traits>;
    using algorithm_match = diff_match_patch_match<all_traits>;
    using algorithm_patch = diff_match_patch_patch<all_traits>;

    using settings_t = types::settings;

    using string_pool_t = typename algorithm_diff::string_pool_t;

    using string_view_t   = typename all_traits::string_traits::string_view_t;
    using owning_string_t = typename all_traits::string_traits::owning_string_t;
    using clock_t         = typename all_traits::clock_traits::type;


    using Operation = typename algorithm_diff::Operation;
    using diff_t    = typename algorithm_diff::diff_t;
    using diffs_t   = typename algorithm_diff::diffs_t;

    using patch_t        = typename algorithm_patch::patch_t;
    using patches_t      = typename algorithm_patch::patches_t;
    using patch_result_t = typename algorithm_patch::patch_result_t;


public:
    constexpr diff_match_patch_base() noexcept = default;
};
}  // namespace dmp

#endif
