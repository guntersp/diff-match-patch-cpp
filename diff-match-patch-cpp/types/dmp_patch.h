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


#ifndef DIFF_MATCH_PATCH_PATCH_H
#define DIFF_MATCH_PATCH_PATCH_H


#include "types/dmp_diff.h"
#include "utils/dmp_encoding.h"

namespace dmp {
namespace types {


/**
 * Class representing one patch Operation::
 */
template <typename _diffs_t>
struct patch {
    using diffs_t = _diffs_t;
    using diff_t  = typename diffs_t::diff_t;

    using string_view_t = typename diff_t::string_view_t;
    using char_traits   = typename diff_t::char_traits;

    diffs_t diffs;

    size_t start1  = 0;
    size_t start2  = 0;
    size_t length1 = 0;
    size_t length2 = 0;

    constexpr patch() noexcept = default;


    inline constexpr bool isNull() const noexcept { return start1 == 0 && start2 == 0 && length1 == 0 && length2 == 0 && diffs.size() == 0; }

    /**
     * Emulate GNU diff's format.
     * Header: @@ -382,8 +481,9 @@
     * Indices are printed as 1-based, not 0-based.
     * @return The GNU diff string.
     */
    template <typename Stream>
    inline constexpr Stream& toString(Stream& s) const noexcept {
        using namespace dmp::utils;

        char_traits::write(s, "@@ -");

        if (length1 == 0) {
            writeToStream(s, start1);
            char_traits::write(s, ",0");
        } else if (length1 == 1) {
            writeToStream(s, start1 + 1);
        } else {
            writeToStream(s, start1 + 1);
            char_traits::write(s, ',');
            writeToStream(s, length1);
        }

        char_traits::write(s, " +");

        if (length2 == 0) {
            writeToStream(s, start2);
            char_traits::write(s, ",0");
        } else if (length2 == 1) {
            writeToStream(s, start2 + 1);
        } else {
            writeToStream(s, start2 + 1);
            char_traits::write(s, ',');
            writeToStream(s, length2);
        }

        char_traits::write(s, " @@\n");
        // Escape the body of the patch with %xx notation.
        for (auto& d : diffs) {
            switch (d.operation) {
                case Operation::INSERT:
                    char_traits::write(s, '+');
                    break;
                case Operation::DELETE:
                    char_traits::write(s, '-');
                    break;
                case Operation::EQUAL:
                    char_traits::write(s, ' ');
                    break;
            }

            encodeURI<char_traits>(s, d.text);
            char_traits::write(s, char_traits::eol);
        }

        return s;
    }
};

}  // namespace types
}  // namespace dmp

#endif
