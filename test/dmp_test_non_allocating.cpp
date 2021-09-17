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


#include <catch2/catch.hpp>

#define DISABLE_VERY_LONG_STRING_TEST
#define DONT_INCLUDE_STD_STRINGVIEW
#define NON_ALLOCATING_MAX_STRING_SIZE 1152

#ifndef STR
// this converts latin1 char strings to the string type used in the test. Since the string might be used outside of a function argument the temp
// string needs to be stored in a pool
#    define STR(S) string_view_t(S, sizeof(S) - 1)
#endif


#include "dmp/traits/dmp_clock_traits_chrono.h"
#include "dmp/traits/dmp_container_traits_non_allocating.h"
#include "dmp/traits/dmp_string_traits_non_allocating.h"

#include "dmp_test_non_allocating_interface.h"
#include "dmp_tests.h"


using traits = dmp::all_traits<dmp::non_allocating_string_traits, dmp::chrono_clock_traits, dmp::nonallocating_container_traits>;


DEFINE_TEST(non_allocating, DiffMatchPatch_diff, commonPrefixTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, commonSuffixTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, commonOverlapTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, halfmatchTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, linesToCharsTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, charsToLinesTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, cleanupMergeTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, cleanupSemanticLosslessTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, cleanupSemanticTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, cleanupEfficiencyTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, prettyHtmlTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, textTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, deltaTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, xIndexTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, levenshteinTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, bisectTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_diff, mainTest)

DEFINE_TEST(non_allocating, DiffMatchPatch_match, alphabetTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_match, bitapTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_match, mainTest)

DEFINE_TEST(non_allocating, DiffMatchPatch_patch, patchObjTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, fromTextTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, toTextTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, addContextTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, makeTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, splitMaxTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, addPaddingTest)
DEFINE_TEST(non_allocating, DiffMatchPatch_patch, applyTest)
