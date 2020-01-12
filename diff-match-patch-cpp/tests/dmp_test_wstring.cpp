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


#include "gtest/gtest.h"

//#define DISABLE_VERY_LONG_STRING_TEST


#include "traits/dmp_clock_traits_chrono.h"
#include "traits/dmp_container_traits_std.h"
#include "traits/dmp_string_traits_wstring.h"

#include "tests/dmp_test_interface.h"
#include "tests/dmp_tests.h"


namespace {
using traits = dmp::all_traits<dmp::std_wstring_traits, dmp::chrono_clock_traits, dmp::std_container_traits>;


DEFINE_GTEST(wstring, DiffMatchPatch_diff, commonPrefixTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, commonSuffixTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, commonOverlapTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, halfmatchTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, linesToCharsTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, charsToLinesTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, cleanupMergeTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, cleanupSemanticLosslessTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, cleanupSemanticTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, cleanupEfficiencyTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, prettyHtmlTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, textTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, deltaTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, xIndexTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, levenshteinTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, bisectTest)
DEFINE_GTEST(wstring, DiffMatchPatch_diff, mainTest)

DEFINE_GTEST(wstring, DiffMatchPatch_match, alphabetTest)
DEFINE_GTEST(wstring, DiffMatchPatch_match, bitapTest)
DEFINE_GTEST(wstring, DiffMatchPatch_match, mainTest)

DEFINE_GTEST(wstring, DiffMatchPatch_patch, patchObjTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, fromTextTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, toTextTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, addContextTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, makeTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, splitMaxTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, addPaddingTest)
DEFINE_GTEST(wstring, DiffMatchPatch_patch, applyTest)

}  // namespace


#ifndef DONT_DEFINE_DIFF_TEST_MAIN
#    include "src/gtest-all.cc"
#    include "src/gtest_main.cc"
#endif


/*
Compile instructions for LLVM:
clang++ -O2 -o diff_match_patch -I diff-match-patch -I googletest\include -I googletest -std=c++17 diff-match-patch\tests\dmp_test_wstring.cpp

Compile instructions for LLVM on windows:
clang-cl" /O2 /Idiff-match-patch /Igoogletest\include /Igoogletest /std:c++17 diff-match-patch\tests\dmp_test_wstring.cpp

dmp_test_wstring.exe
*/
