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

#include "diff_match_patch.h"

#include <fstream>


namespace {
using traits = dmp::all_traits<dmp::std_wstring_traits, dmp::chrono_clock_traits, dmp::std_container_traits>;
using dmp_t  = dmp::diff_match_patch<traits>;
}  // namespace


#if 1  // ndef DONT_DEFINE_DIFF_TEST_MAIN
int main(int /*argc*/, char** /*argv*/) {
    std::wstring text1;
    std::wstring text2;

    {
        std::wifstream f("Speedtest1.txt", std::ios::binary);
        if (!f) {
            return 1;
        }

        text1 = std::wstring((std::istreambuf_iterator<wchar_t>(f)), std::istreambuf_iterator<wchar_t>());
    }
    {
        std::wifstream f("Speedtest2.txt", std::ios::binary);
        if (!f) {
            return 2;
        }

        text2 = std::wstring((std::istreambuf_iterator<wchar_t>(f)), std::istreambuf_iterator<wchar_t>());
    }

    std::cout << "input lengths: " << text1.length() << " " << text2.length() << "\n";

    dmp_t dmp;
    dmp.Diff_Timeout = 0;


    // Execute one reverse diff as a warmup.
    dmp.diff_main(text2, text1);

    {
        auto ms_start(dmp_t::clock_t::now());
        auto diffs(dmp.diff_main(text1, text2));
        auto ms_end(dmp_t::clock_t::now());


        std::cout << "Elapsed time: " << ms_start.mSecsTo(ms_end) << " [ms]"
                  << "\n";


#    if 0
    std::cout << "diffs: " << diffs.size() << "\n";
    dmp.diff_prettyHtml(std::wcout, diffs);
#    endif
    }

#    if 1
    {
        auto ms_start(dmp_t::clock_t::now());
        auto patch(dmp.patch_make(text1, text2));
        auto ms_end(dmp_t::clock_t::now());


        std::cout << "Elapsed time for patch creation: " << ms_start.mSecsTo(ms_end) << " [ms]"
                  << "\n";

#        if 0
    std::cout << "patch: " << patch.size() << "\n";
    dmp.patch_toText(std::wcout, patch);
#        endif
    }
#    endif


    return 0;
}
#endif

/*
Compile instructions for LLVM:
clang++ -O2 -o diff_match_patch -I diff-match-patch -std=c++17 diff-match-patch\tests\dmp_speedtest_wstring.cpp

Compile instructions for LLVM on windows:
clang-cl" /O2 /Idiff-match-patch /std:c++17 diff-match-patch\tests\dmp_speedtest_wstring.cpp

dmp_test_wstring.exe
*/
