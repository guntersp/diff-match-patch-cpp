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


#ifndef DIFF_MATCH_PATCH_SETTINGS_H
#define DIFF_MATCH_PATCH_SETTINGS_H


namespace dmp {
namespace types {

struct settings {

    // Defaults.
    // Set these on your diff_match_patch instance to override the defaults.

    // Number of seconds to map a diff before giving up (0 for infinity).
    float Diff_Timeout = 1.0f;
    // Cost of an empty edit operation in terms of edit characters.

    short Diff_EditCost = 4;
    // At what point is no match declared (0.0 = perfection, 1.0 = very loose).

    float Match_Threshold = 0.5f;

    // How far to search for a match (0 = exact location, 1000+ = broad match).
    // A match this many characters away from the expected location will add
    // 1.0 to the score (0.0 is a perfect match).
    int Match_Distance = 1000;

    // When deleting a large block of text (over ~64 characters), how close
    // do the contents have to be to match the expected contents. (0.0 =
    // perfection, 1.0 = very loose).  Note that Match_Threshold controls
    // how closely the end points of a delete need to match.
    float Patch_DeleteThreshold = 0.5f;

    // Chunk size for context length.
    short Patch_Margin = 4;

    // The number of bits in an int.
    short Match_MaxBits = 32;

public:
    constexpr settings() noexcept = default;
};

}  // namespace types
}  // namespace dmp

#endif
