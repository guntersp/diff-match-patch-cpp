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


#ifndef DIFF_MATCH_PATCH_STRINGTRAITS_WSTRING_H
#define DIFF_MATCH_PATCH_STRINGTRAITS_WSTRING_H


#include "dmp/traits/dmp_default_char_traits.h"
#include "dmp/utils/dmp_stringpool.h"
#include "dmp/utils/dmp_stringwrapper.h"
#include "dmp/utils/dmp_stringwrapper_utils.h"
#include "dmp/utils/dmp_traits.h"

#include <cwctype>
#include <sstream>
#include <string>
#include <string_view>


namespace dmp {
namespace traits {

template <>
struct string_traits<std::wstring> {
    using char_t      = wchar_t;
    using char_traits = traits::default_char_traits<char_t>;

    using owning_string_t              = std::wstring;
    using string_view_t                = utils::string_wrapper<std::wstring_view, char_t>;
    static constexpr const size_t npos = std::wstring::npos;

    using stringstream_t = std::wstringstream;

    using encoding_char_t = dmp::traits::string_encoding_char_t;

    template <typename string_view_t, typename owning_string_t, template <typename> class list_t>
    using string_pool_t = utils::string_pool<string_view_t, owning_string_t, list_t>;
};


}  // namespace traits

using std_wstring_traits = traits::string_traits<std::wstring>;

}  // namespace dmp

#endif
