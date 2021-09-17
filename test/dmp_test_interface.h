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


#ifndef DIFF_MATCH_PATCH_TEST_INTERFACE_H
#define DIFF_MATCH_PATCH_TEST_INTERFACE_H


#include "dmp/diff_match_patch.h"

#include "dmp/traits/dmp_encoding_sub_traits.h"
#include "dmp/utils/dmp_stringwrapper_utils.h"
#include "dmp/utils/dmp_utils.h"

#include <vector>

namespace dmp {
namespace tests {

template <typename type>
struct vector : public std::vector<type> {
    using parent = std::vector<type>;

    using parent::parent;
};


template <typename all_traits>
struct diff_match_patch_test : public diff_match_patch<all_traits> {
    using parent = diff_match_patch<all_traits>;

    using commons         = diff_match_patch_common<all_traits>;
    using algorithm_diff  = diff_match_patch_diff<all_traits>;
    using algorithm_match = diff_match_patch_match<all_traits>;
    using algorithm_patch = diff_match_patch_patch<all_traits>;

    using settings_t = typename parent::settings_t;

    using string_pool_t = typename parent::string_pool_t;

    using string_view_t        = typename parent::string_view_t;
    using owning_string_t      = typename parent::owning_string_t;
    using long_owning_string_t = typename parent::owning_string_t;
    using stringstream_t       = typename parent::stringstream_t;
    using clock_t              = typename parent::clock_t;

    using diffs_t = typename parent::diffs_t;
    using Diffs   = typename parent::Diffs;

    using patches_t = typename parent::patches_t;
    using Patches   = typename parent::Patches;

    using Operation   = typename parent::Operation;
    using Diff        = typename parent::diff_t;
    using Patch       = typename parent::patch_t;
    using PatchResult = typename parent::patch_result_t;


    using string_traits = typename algorithm_diff::string_traits;
    using char_traits   = typename algorithm_diff::char_traits;
    using char_t        = typename algorithm_diff::char_t;


    using encoding_algorithm_diff_t = typename parent::encoding_algorithm_diff_t;

    using encoding_string_view_t   = typename parent::encoding_string_view_t;
    using encoding_owning_string_t = typename parent::encoding_owning_string_t;
    using encoding_list_t          = typename parent::encoding_list_t;
    using encoding_char_t          = typename parent::encoding_char_t;

    using ediff_t  = typename parent::encoding_diff_t;
    using ediffs_t = typename parent::encoding_diffs_t;

public:
    constexpr diff_match_patch_test() noexcept = default;

    /**
     * Determine the common prefix of two strings.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the start of each string.
     */
    inline constexpr auto diff_commonPrefix(string_view_t text1, string_view_t text2) const noexcept {
        return commons::diff_commonPrefix(text1, text2);
    }

    /**
     * Determine the common suffix of two strings.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the end of each string.
     */
    inline constexpr auto diff_commonSuffix(string_view_t text1, string_view_t text2) const noexcept {
        return commons::diff_commonSuffix(text1, text2);
    }


    /**
     * Determine if the suffix of one string is the prefix of another.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the end of the first
     *     string and the start of the second string.
     */
    inline constexpr size_t diff_commonOverlap(string_view_t text1, string_view_t text2) const noexcept {
        return commons::diff_commonOverlap(text1, text2);
    }

    template <typename String1, typename String2>
    inline constexpr auto concat(string_pool_t& pool, const String1& s1, const String2& s2) const noexcept {
        return pool.create(s1, s2);
    }


    using HalfMatchResult = typename parent::HalfMatchResult;
    inline constexpr HalfMatchResult diff_halfMatch(string_view_t text1, string_view_t text2) const noexcept {
        HalfMatchResult hm;
        parent::diff_halfMatch(*this, hm, text1, text2);
        return hm;
    }
    inline constexpr bool diff_halfMatchOriginalReturn(string_view_t text1, string_view_t text2) const noexcept {
        HalfMatchResult hm;
        return parent::diff_halfMatch(*this, hm, text1, text2);
    }

    inline constexpr void diff_linesToChars(encoding_owning_string_t (&encodedStrings)[2], string_view_t text1, string_view_t text2,
                                            encoding_list_t& lines) const noexcept {
        parent::diff_linesToChars(encodedStrings, text1, text2, lines);
    }

    inline constexpr void diff_charsToLines(diffs_t& diffs, string_pool_t& storage, const ediffs_t& eDiffs, const encoding_list_t& lines) const
        noexcept {
        parent::diff_charsToLines(diffs, storage, eDiffs, lines);
    }

    inline constexpr Diffs diff_bisect(string_view_t text1, string_view_t text2, clock_t deadline) const noexcept {
        Diffs container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        container.stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::diff_bisect(*this, *container.elements, container.stringPool, text1, text2, deadline);
        container.stringPool.resetOriginalTexts();
        return container;
    }


    inline static constexpr void diff_rebuildtexts(const Diffs& diffs, owning_string_t& text1, owning_string_t& text2) noexcept {
        using namespace dmp::utils;

        stringstream_t s1;
        stringstream_t s2;

        for (auto& d : diffs) {
            if (d.operation != Operation::INSERT) {
                writeToStream(s1, d.text);
            }
            if (d.operation != Operation::DELETE) {
                writeToStream(s2, d.text);
            }
        }

        text1 = s1.str();
        text2 = s2.str();
    }

    template <typename map_t>
    inline constexpr void match_alphabet(string_view_t pattern, map_t& s) const noexcept {
        parent::match_alphabet(pattern, s);
    }

    inline constexpr size_t match_bitap(string_view_t text, string_view_t pattern, int loc) noexcept {
        return parent::match_bitap(*this, text, pattern, static_cast<size_t>(loc));
    }

    inline constexpr owning_string_t toString(const Patch& patch) const noexcept {
        stringstream_t s;
        patch.toString(s);
        return s.str();
    }


    /**
     * Increase the context until it is unique,
     * but don't let the pattern expand beyond Match_MaxBits.
     * @param patch The patch to grow.
     * @param text Source text.
     */
public:
    using parent ::patch_addContext;
    inline constexpr void patch_addContext(Patch& patch, string_view_t text) const noexcept { patch_addContext(*this, patch, text); }


    /**
     * Look through the patches and break up any which are longer than the
     * maximum limit of the match algorithm.
     * Intended to be called only from within patch_apply.
     * @param patches List of Patch objects.
     */
public:
    using parent ::patch_splitMax;
    inline constexpr void patch_splitMax(Patches& patches) const noexcept { parent::patch_splitMax(*this, *patches.elements, patches.stringPool); }


    /**
     * Add some padding on text start and end so that edges can match something.
     * Intended to be called only from within patch_apply.
     * @param patches Array of Patch objects.
     * @return The padding string added to each side.
     */
public:
    using parent ::patch_addPadding;
    inline constexpr string_view_t patch_addPadding(Patches& patches) const noexcept {
        return parent::patch_addPadding(*this, *patches.elements, patches.stringPool);
    }

    inline constexpr owning_string_t toString(const PatchResult& pr) const noexcept {
        using namespace dmp::utils;

        stringstream_t s;
        writeToStream(s, pr.text2);

        for (auto r : pr.results) {
            char_traits::write(s, char_traits::tab);

            char_traits::write(s, r ? "True" : "False");
        }

        return s.str();
    }
};

}  // namespace tests
}  // namespace dmp

#endif
