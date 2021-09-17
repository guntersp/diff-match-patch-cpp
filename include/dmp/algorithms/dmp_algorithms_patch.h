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


#ifndef DIFF_MATCH_PATCH_ALGORITHMS_PATCH_H
#define DIFF_MATCH_PATCH_ALGORITHMS_PATCH_H


#include "dmp/algorithms/dmp_algorithms_common.h"
#include "dmp/algorithms/dmp_algorithms_diff.h"
#include "dmp/algorithms/dmp_algorithms_match.h"
#include "dmp/types/dmp_patch.h"
#include "dmp/types/dmp_settings.h"
#include "dmp/utils/dmp_stringpool_base.h"


namespace dmp {


/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */
template <class all_traits>
struct diff_match_patch_patch {
    using commons   = diff_match_patch_common<all_traits>;
    using dmp_diff  = diff_match_patch_diff<all_traits>;
    using dmp_match = diff_match_patch_match<all_traits>;

    using settings_t = typename commons::settings_t;

    using string_traits    = typename all_traits::string_traits;
    using container_traits = typename all_traits::container_traits;

    using char_traits     = typename string_traits::char_traits;
    using string_view_t   = typename string_traits::string_view_t;
    using owning_string_t = typename string_traits::owning_string_t;
    using char_t          = typename string_traits::char_t;

    static constexpr const size_t npos = string_traits::npos;

    using string_pool_t = typename commons::string_pool_t;


    using Operation = typename commons::Operation;

    using diff_t  = typename commons::diff_t;
    using diffs_t = typename commons::diffs_t;

    using patch_t = types::patch<diffs_t>;
    struct patches_t : public container_traits::template patches_list<patch_t> {
        using parent    = typename container_traits::template patches_list<patch_t>;
        using element_t = patch_t;

        using parent::parent;


        inline constexpr patches_t(const patches_t& o) noexcept : parent{} {
            parent::reserve(o.size());
            for (auto& d : o) {
                parent::push_back(d);
            }
        }


        inline constexpr patches_t& operator=(const patches_t& o) noexcept {
            if (this != &o) {
                parent::clear();
                parent::reserve(o.size());
                for (auto& d : o) {
                    parent::push_back(d);
                }
            }
            return *this;
        }
    };

    using patch_result_list_t = typename container_traits::template patch_result_list<bool>;
    struct patch_result_t {
        string_view_t       text2;
        patch_result_list_t results;
    };


public:
    constexpr diff_match_patch_patch() noexcept = default;


    /**
     * Increase the context until it is unique,
     * but don't let the pattern expand beyond Match_MaxBits.
     * @param patch The patch to grow.
     * @param text Source text.
     */
protected:
    inline static constexpr void patch_addContext(const settings_t& settings, patch_t& patch, string_view_t text) noexcept {
        if (text.length() == 0) {
            return;
        }

        using namespace dmp::utils;


        string_view_t pattern = text.substring(patch.start2, patch.length1);
        int           padding = 0;

        // Look for the first and last matches of pattern in text.  If two
        // different matches are found, increase the pattern length.
        while (text.indexOf(pattern) != text.lastIndexOf(pattern)
               && pattern.length() < static_cast<size_t>(settings.Match_MaxBits - settings.Patch_Margin - settings.Patch_Margin)) {
            padding += settings.Patch_Margin;
            auto s(static_cast<size_t>(max(0, static_cast<int>(patch.start2) - padding)));
            auto e(static_cast<size_t>(
                min(static_cast<int>(text.length()), static_cast<int>(patch.start2) + static_cast<int>(patch.length1) + padding)));
            pattern = text.substring(s, e - s);
        }
        // Add one chunk for good luck.
        padding += settings.Patch_Margin;

        // Add the prefix.
        auto          s(static_cast<size_t>(max(0, static_cast<int>(patch.start2) - padding)));
        string_view_t prefix = text.substring(s, patch.start2 - s);
        if (prefix.length() != 0) {
            patch.diffs.push_front(diff_t(Operation::EQUAL, prefix));
        }
        // Add the suffix.
        s                    = patch.start2 + patch.length1;
        string_view_t suffix = text.substring(
            s, static_cast<size_t>(min(static_cast<int>(text.length()), static_cast<int>(patch.start2) + static_cast<int>(patch.length1) + padding))
                   - s);
        if (suffix.length() != 0) {
            patch.diffs.push_back(diff_t(Operation::EQUAL, suffix));
        }

        // Roll back the start points.
        patch.start1 -= prefix.length();
        patch.start2 -= prefix.length();
        // Extend the lengths.
        patch.length1 += prefix.length() + suffix.length();
        patch.length2 += prefix.length() + suffix.length();
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * A set of diffs will be computed.
     * @param text1 Old text.
     * @param text2 New text.
     * @return List of Patch objects.
     */
public:
    inline static constexpr bool patch_make(const settings_t& settings, patches_t& patches, string_pool_t& pool, string_view_t text1,
                                            string_view_t text2) noexcept {
        patches.clear();

        // Check for null inputs not needed since null can't be passed in C#.
        // No diffs provided, compute our own.
        diffs_t diffs;
        dmp_diff::diff_main(settings, diffs, pool, text1, text2, true);
        if (diffs.size() > 2) {
            dmp_diff::diff_cleanupSemantic(diffs, pool);
            dmp_diff::diff_cleanupEfficiency(settings, diffs, pool);
        }
        return patch_make(settings, patches, pool, text1, diffs);
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * text1 will be derived from the provided diffs.
     * @param diffs Array of Diff objects for text1 to text2.
     * @return List of Patch objects.
     */
public:
    inline static constexpr bool patch_make(const settings_t& settings, patches_t& patches, string_pool_t& pool, const diffs_t& diffs) noexcept {
        // Check for null inputs not needed since null can't be passed in C#.
        // No origin string provided, compute our own.
        patches.clear();

        string_view_t text1(commons::diff_text1(pool, diffs));
        return patch_make(settings, patches, pool, text1, diffs);
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * text2 is ignored, diffs are the delta between text1 and text2.
     * @param text1 Old text
     * @param text2 Ignored.
     * @param diffs Array of Diff objects for text1 to text2.
     * @return List of Patch objects.
     * @deprecated Prefer patch_make(string text1, List<Diff> diffs).
     */
public:
    inline static constexpr bool patch_make(const settings_t& settings, patches_t& patches, string_pool_t& pool, string_view_t text1,
                                            string_view_t /*text2*/, const diffs_t& diffs) noexcept {
        return patch_make(settings, patches, pool, text1, diffs);
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * text2 is not provided, diffs are the delta between text1 and text2.
     * @param text1 Old text.
     * @param diffs Array of Diff objects for text1 to text2.
     * @return List of Patch objects.
     */
public:
    inline static constexpr bool patch_make(const settings_t& settings, patches_t& patches, string_pool_t& pool, string_view_t text1,
                                            const diffs_t& diffs) noexcept {

        using namespace dmp::utils;


        // Check for null inputs not needed since null can't be passed in C#.
        patches.clear();
        if (diffs.size() == 0) {
            return true;  // Get rid of the null case.
        }
        patch_t patch;
        int     char_count1 = 0;  // Number of characters into the text1 string.
        int     char_count2 = 0;  // Number of characters into the text2 string.
        // Start with text1 (prepatch_text) and apply the diffs until we arrive at
        // text2 (postpatch_text). We recreate the patches one by one to determine
        // context info.

        string_view_t prepatch_text = text1;


        size_t index = 0;

        size_t n = diffs.size();

        while (index < n) {
            // calculate length of next string
            size_t len = prepatch_text.length();
            size_t maxLength(len);

            size_t i(index);
            while (i < n) {
                auto& d(diffs[i]);

                i++;


                switch (d.operation) {
                    case Operation::INSERT:
                        len += d.text.length();
                        break;
                    case Operation::DELETE:
                        len -= min(len, d.text.length());
                        break;
                    case Operation::EQUAL:
                        if (d.text.length() >= static_cast<size_t>(2 * settings.Patch_Margin)) {
                            // Time for a new patch.
                            if (patch.diffs.size() != 0) {
                                i = n;  // break the loop
                            }
                        }
                        break;
                }

                if (len > maxLength) {
                    maxLength = len;
                }
            }


            auto postpatch_text(pool.createNext(maxLength));
            if (!postpatch_text) {
                return false;
            }

            auto p(postpatch_text);
            auto l(maxLength);


            str_copy(p, l, prepatch_text);
            l = maxLength - l;


            while (index < n && len > 0) {
                auto& aDiff(diffs[index]);

                index++;


                if (patch.diffs.size() == 0 && aDiff.operation != Operation::EQUAL) {
                    // A new patch starts here.
                    patch.start1 = static_cast<size_t>(char_count1);
                    patch.start2 = static_cast<size_t>(char_count2);
                }


                switch (aDiff.operation) {
                    case Operation::INSERT:
                        patch.diffs.push_back(aDiff);
                        patch.length2 += aDiff.text.length();
                        l += str_insert(postpatch_text, l, static_cast<size_t>(char_count2), aDiff.text);
                        break;
                    case Operation::DELETE:
                        patch.length1 += aDiff.text.length();
                        patch.diffs.push_back(aDiff);
                        l -= str_remove(postpatch_text, l, static_cast<size_t>(char_count2), aDiff.text.length());
                        break;
                    case Operation::EQUAL:
                        if (aDiff.text.length() <= static_cast<size_t>(2 * settings.Patch_Margin) && patch.diffs.size() != 0 && aDiff != diffs.back()) {
                            // Small equality inside a patch.
                            patch.diffs.push_back(aDiff);
                            patch.length1 += aDiff.text.length();
                            patch.length2 += aDiff.text.length();
                        }

                        if (aDiff.text.length() >= static_cast<size_t>(2 * settings.Patch_Margin)) {
                            // Time for a new patch.
                            if (patch.diffs.size() != 0) {
                                patch_addContext(settings, patch, prepatch_text);
                                patches.push_back(patch);
                                patch = patch_t();
                                // Unlike Unidiff, our patch lists have a rolling context.
                                // https://github.com/google/diff-match-patch/wiki/Unidiff
                                // Update prepatch text & pos to reflect the application of the
                                // just completed patch.
                                prepatch_text = string_view_t { postpatch_text, l };
                                char_count1   = char_count2;
                                len           = 0;  // break the loop
                            }
                        }
                        break;
                }

                // Update the current character count.
                if (aDiff.operation != Operation::INSERT) {
                    char_count1 += static_cast<int>(aDiff.text.length());
                }
                if (aDiff.operation != Operation::DELETE) {
                    char_count2 += static_cast<int>(aDiff.text.length());
                }
            }
        }


        // Pick up the leftover patch if not empty.
        if (patch.diffs.size() != 0) {
            patch_addContext(settings, patch, prepatch_text);
            patches.push_back(patch);
        }

        return true;
    }


    /**
     * Merge a set of patches onto the text.  Return a patched text, as well
     * as an array of true/false values indicating which patches were applied.
     * @param patches Array of Patch objects
     * @param text Old text.
     * @return Two element Object array, containing the new text and an array of
     *      bool values.
     */
public:
    inline static constexpr patch_result_t patch_apply(const settings_t& settings, const patches_t& _patches, string_pool_t& pool,
                                                       string_view_t _text) noexcept {
        if (_patches.size() == 0) {
            return { _text, {} };
        }

        using namespace dmp::utils;

        // Deep copy the patches so that no changes are made to originals.
        auto patches(_patches);

        patch_result_t result;

        auto nullPadding = patch_addPadding(settings, patches, pool);

        auto& text(result.text2);
        text = pool.appended(nullPadding, _text, nullPadding);

        patch_splitMax(settings, patches, pool);


        int x = 0;
        // delta keeps track of the offset between the expected and actual
        // location of the previous patch.  If there are patches expected at
        // positions 10 and 20, but the first patch was found at 12, delta is 2
        // and the second patch has an effective expected position of 22.
        int   delta = 0;
        auto& results(result.results);
        results.resize(patches.size());
        for (auto& aPatch : patches) {
            size_t expected_loc = static_cast<size_t>(max(0, static_cast<int>(aPatch.start2) + delta));
            auto   text1        = commons::diff_text1(pool, aPatch.diffs);
            size_t start_loc {};
            size_t end_loc = npos;
            if (text1.length() > static_cast<size_t>(settings.Match_MaxBits)) {
                // patch_splitMax will only provide an oversized pattern
                // in the case of a monster delete.
                start_loc = dmp_match::match_main(settings, text, text1.substring(0, static_cast<size_t>(settings.Match_MaxBits)), expected_loc);
                if (start_loc != npos) {
                    end_loc = dmp_match::match_main(settings, text, text1.substring(text1.length() - static_cast<size_t>(settings.Match_MaxBits)),
                                                    expected_loc + text1.length() - static_cast<size_t>(settings.Match_MaxBits));
                    if (end_loc == npos || start_loc >= end_loc) {
                        // Can't find valid trailing context.  Drop this patch.
                        start_loc = npos;
                    }
                }
            } else {
                start_loc = dmp_match::match_main(settings, text, text1, expected_loc);
            }
            if (start_loc == npos) {
                // No match found.  :(
                results[static_cast<size_t>(x)] = false;
                // Subtract the delta for this failed patch from subsequent patches.
                delta -= static_cast<int>(aPatch.length2) - static_cast<int>(aPatch.length1);
            } else {
                // Found a match.  :)
                results[static_cast<size_t>(x)] = true;
                delta      = static_cast<int>(start_loc) - static_cast<int>(expected_loc);
                string_view_t text2;
                if (end_loc == npos) {
                    text2 = text.substring(start_loc, min(start_loc + text1.length(), text.length()) - start_loc);
                } else {
                    text2 = text.substring(start_loc, min(end_loc + static_cast<size_t>(settings.Match_MaxBits), text.length()) - start_loc);
                }
                if (text1 == text2) {
                    // Perfect match, just shove the Replacement text in.
                    text = pool.appended(text.substring(0, start_loc), commons::diff_text2(pool, aPatch.diffs),
                                         text.substring(start_loc + text1.length()));
                } else {
                    // Imperfect match.  Run a diff to get a framework of equivalent
                    // indices.
                    diffs_t diffs;
                    dmp_diff::diff_main(settings, diffs, pool, text1, text2, false);
                    if (text1.length() > static_cast<size_t>(settings.Match_MaxBits)
                        && static_cast<float>(commons::diff_levenshtein(diffs)) / static_cast<float>(text1.length()) > settings.Patch_DeleteThreshold) {
                        // The end points match, but the content is unacceptably bad.
                        results[static_cast<size_t>(x)] = false;
                    } else {
                        dmp_diff::diff_cleanupSemanticLossless(diffs, pool);

                        size_t len = text.length();
                        size_t maxLength(len);
                        size_t index1 = 0;
                        for (auto& aDiff : aPatch.diffs) {
                            if (aDiff.operation != Operation::EQUAL) {
                                size_t index2 = dmp_diff::diff_xIndex(diffs, index1);
                                if (aDiff.operation == Operation::INSERT) {
                                    // Insertion
                                    len += aDiff.text.length();
                                } else if (aDiff.operation == Operation::DELETE) {
                                    // Deletion
                                    len -= dmp_diff::diff_xIndex(diffs, index1 + aDiff.text.length()) - index2;
                                }
                                if (len > maxLength) {
                                    maxLength = len;
                                }
                            }
                            if (aDiff.operation != Operation::DELETE) {
                                index1 += aDiff.text.length();
                            }
                        }


                        auto s(pool.createNext(maxLength));
                        if (!s) {
                            return {};
                        }

                        auto   p(s);
                        size_t l(maxLength);
                        str_copy(p, l, text);

                        l = maxLength - l;

                        index1 = 0;
                        for (auto& aDiff : aPatch.diffs) {
                            if (aDiff.operation != Operation::EQUAL) {
                                size_t index2 = dmp_diff::diff_xIndex(diffs, index1);
                                if (aDiff.operation == Operation::INSERT) {
                                    // Insertion
                                    l += str_insert(s, l, start_loc + index2, aDiff.text);
                                } else if (aDiff.operation == Operation::DELETE) {
                                    // Deletion
                                    l -= str_remove(s, l, start_loc + index2, dmp_diff::diff_xIndex(diffs, index1 + aDiff.text.length()) - index2);
                                }
                            }
                            if (aDiff.operation != Operation::DELETE) {
                                index1 += aDiff.text.length();
                            }
                        }

                        text = string_view_t { s, l };
                    }
                }
            }
            x++;
        }
        // Strip the padding off.
        text = text.substring(nullPadding.length(), text.length() - 2 * nullPadding.length());
        return result;
    }


    /**
     * Add some padding on text start and end so that edges can match something.
     * Intended to be called only from within patch_apply.
     * @param patches Array of Patch objects.
     * @return The padding string added to each side.
     */
public:
    inline static constexpr string_view_t patch_addPadding(const settings_t& settings, patches_t& patches, string_pool_t& pool) noexcept {
        size_t paddingLength(static_cast<size_t>(settings.Patch_Margin));

        using namespace dmp::utils;

        auto np(pool.createNext(paddingLength));
        if (!np) {
            return {};
        }

        string_view_t nullPadding { np, paddingLength };
        for (size_t x = 1; x <= paddingLength; x++) {
            *np++ = char_traits::cast(x);
        }

        // Bump all the patches forward.
        for (auto& aPatch : patches) {
            aPatch.start1 += paddingLength;
            aPatch.start2 += paddingLength;
        }

        // Add some padding on start of first diff.
        {
            auto& patch(patches[0]);
            auto& diffs = patch.diffs;
            if (diffs.size() == 0 || diffs[0].operation != Operation::EQUAL) {
                // Add nullPadding equality.
                diffs.push_front(diff_t(Operation::EQUAL, nullPadding));
                patch.start1 -= paddingLength;  // Should be 0.
                patch.start2 -= paddingLength;  // Should be 0.
                patch.length1 += paddingLength;
                patch.length2 += paddingLength;
            } else if (paddingLength > diffs[0].text.length()) {
                // Grow first equality.
                auto&  firstDiff(diffs[0]);
                size_t extraLength = paddingLength - firstDiff.text.length();
                firstDiff.text     = pool.appended(nullPadding.substring(firstDiff.text.length()), firstDiff.text);
                patch.start1 -= extraLength;
                patch.start2 -= extraLength;
                patch.length1 += extraLength;
                patch.length2 += extraLength;
            }
        }

        // Add some padding on end of last diff.
        {
            auto& patch(patches.back());
            auto& diffs = patch.diffs;
            if (diffs.size() == 0 || diffs.back().operation != Operation::EQUAL) {
                // Add nullPadding equality.
                diffs.push_back(diff_t(Operation::EQUAL, nullPadding));
                patch.length1 += paddingLength;
                patch.length2 += paddingLength;
            } else if (paddingLength > diffs.back().text.length()) {
                // Grow last equality.
                auto&  lastDiff(diffs.back());
                size_t extraLength = paddingLength - lastDiff.text.length();
                pool.append(lastDiff.text, nullPadding.substring(0, extraLength));
                patch.length1 += extraLength;
                patch.length2 += extraLength;
            }
        }

        return nullPadding;
    }


    /**
     * Look through the patches and break up any which are longer than the
     * maximum limit of the match algorithm.
     * Intended to be called only from within patch_apply.
     * @param patches List of Patch objects.
     */
public:
    inline static constexpr void patch_splitMax(const settings_t& settings, patches_t& patches, string_pool_t& pool) noexcept {
        using namespace dmp::utils;

        short patch_size = settings.Match_MaxBits;
        for (int x = 0; x < static_cast<int>(patches.size()); x++) {
            if (patches[static_cast<size_t>(x)].length1 <= static_cast<size_t>(patch_size)) {
                continue;
            }
            patch_t bigpatch(patches[static_cast<size_t>(x)]);
            // Remove the big old patch.
            patches.splice(static_cast<size_t>(x--), 1);
            int           start1 = static_cast<int>(bigpatch.start1);
            int           start2 = static_cast<int>(bigpatch.start2);
            string_view_t precontext {};
            auto          bps(bigpatch.diffs.size());
            size_t        bpi = 0;
            while (bpi < bps) {
                // Create one of several smaller patches.
                patch_t patch;
                bool    empty = true;
                patch.start1  = static_cast<size_t>(start1) - precontext.length();
                patch.start2  = static_cast<size_t>(start2) - precontext.length();
                if (precontext.length() != 0) {
                    patch.length1 = patch.length2 = precontext.length();
                    patch.diffs.push_back(diff_t(Operation::EQUAL, precontext));
                }
                while (bpi < bps && patch.length1 < static_cast<size_t>(static_cast<int>(patch_size) - settings.Patch_Margin)) {
                    Operation     diff_type = bigpatch.diffs[bpi].operation;
                    string_view_t diff_text = bigpatch.diffs[bpi].text;
                    if (diff_type == Operation::INSERT) {
                        // Insertions are harmless.
                        patch.length2 += diff_text.length();
                        start2 += diff_text.length();
                        patch.diffs.push_back(bigpatch.diffs[bpi]);
                        bpi++;
                        empty = false;
                    } else if (diff_type == Operation::DELETE && patch.diffs.size() == 1 && patch.diffs[0].operation == Operation::EQUAL
                               && diff_text.length() > static_cast<size_t>(2 * static_cast<int>(patch_size))) {
                        // This is a large deletion.  Let it pass in one chunk.
                        patch.length1 += diff_text.length();
                        start1 += diff_text.length();
                        empty = false;
                        patch.diffs.push_back(diff_t(diff_type, diff_text));
                        bpi++;
                    } else {
                        // Deletion or equality.  Only take as much as we can stomach.
                        diff_text = diff_text.substring(size_t(0), static_cast<size_t>(min(static_cast<int>(diff_text.length()), static_cast<int>(patch_size) - static_cast<int>(patch.length1) - settings.Patch_Margin)));
                        patch.length1 += diff_text.length();
                        start1 += diff_text.length();
                        if (diff_type == Operation::EQUAL) {
                            patch.length2 += diff_text.length();
                            start2 += diff_text.length();
                        } else {
                            empty = false;
                        }
                        patch.diffs.push_back(diff_t(diff_type, diff_text));
                        if (diff_text == bigpatch.diffs[bpi].text) {
                            bpi++;
                        } else {
                            bigpatch.diffs[bpi].text = bigpatch.diffs[bpi].text.substring(diff_text.length());
                        }
                    }
                }
                // Compute the head context for the next patch.
                precontext = commons::diff_text2(pool, patch.diffs);
                precontext = precontext.substring(static_cast<size_t>(max(0, static_cast<int>(precontext.length()) - settings.Patch_Margin)));

                string_view_t postcontext {};
                // Append the end context for this patch.
                auto dtext1(commons::diff_text1(pool, bigpatch.diffs, bpi));
                if (dtext1.length() > static_cast<size_t>(settings.Patch_Margin)) {
                    postcontext = dtext1.substring(0, static_cast<size_t>(settings.Patch_Margin));
                } else {
                    postcontext = dtext1;
                }

                if (postcontext.length() != 0) {
                    patch.length1 += postcontext.length();
                    patch.length2 += postcontext.length();
                    if (patch.diffs.size() != 0 && patch.diffs[patch.diffs.size() - 1].operation == Operation::EQUAL) {
                        pool.append(patch.diffs[patch.diffs.size() - 1].text, postcontext);
                    } else {
                        patch.diffs.push_back(diff_t(Operation::EQUAL, postcontext));
                    }
                }
                if (!empty) {
                    patches.splice(static_cast<size_t>(++x), 0, std::move(patch));
                }
            }
        }
    }


    /**
     * Take a list of patches and return a textual representation.
     * @param patches List of Patch objects.
     * @return Text representation of patches.
     */
public:
    template <typename Stream>
    inline static constexpr Stream& patch_toText(Stream& s, const patches_t& patches) noexcept {
        for (auto& p : patches) {
            p.toString(s);
        }
        return s;
    }

    /**
     * Parse a textual representation of patches and return a List of Patch
     * objects.
     * @param textline Text representation of patches.
     * @return List of Patch objects.
     * @throws ArgumentException If invalid input.
     */
public:
    inline static constexpr bool patch_fromText(patches_t& patches, string_pool_t& pool, string_view_t textline) noexcept {
        using namespace dmp::utils;

        patches.clear();


        size_t        len   = textline.length();
        size_t        start = 0;
        string_view_t line;

        if (!commons::nextLine(line, textline, start, len, char_traits::eol)) {
            // empty
            return true;
        }


        while (true) {
            // A replacement for the regexp "^@@ -(\\d+),?(\\d*) \\+(\\d+),?(\\d*) @@$" exact match

            auto t(line.data());
            auto l(line.length());

            auto   sp1(t);
            size_t start1 = 0;
            auto   lp1(t);
            size_t length1 = 0;
            auto   sp2(t);
            size_t start2 = 0;
            auto   lp2(t);
            size_t length2 = 0;

            do {

                if ((l -= 9) > 0 && *t == char_traits::cast('@') && *++t == char_traits::cast('@') && *++t == char_traits::cast(' ')
                    && *++t == char_traits::cast('-') && char_traits::isDigit(*++t)) {
                    sp1 = t;
                    do {
                        start1++;
                    } while (--l > 0 && char_traits::isDigit(*++t));

                    if (l > 0 && *t == char_traits::cast(',')) {
                        ++t;
                        --l;
                    }
                    lp1 = t;
                    while (l > 0 && char_traits::isDigit(*t)) {
                        --l;
                        length1++;
                        t++;
                    }
                    if (l > 0 && *t++ == char_traits::cast(' ') && *t++ == char_traits::cast('+') && char_traits::isDigit(*t)) {
                        sp2 = t;
                        do {
                            start2++;
                            --l;
                        } while (char_traits::isDigit(*++t));

                        if (l > 0 && *t == char_traits::cast(',')) {
                            ++t;
                            --l;
                        }

                        lp2 = t;
                        while (l > 0 && char_traits::isDigit(*t)) {
                            --l;
                            length2++;
                            t++;
                        }

                        if (l == 0 && *t++ == char_traits::cast(' ') && *t++ == char_traits::cast('@') && *t == char_traits::cast('@')) {
                            break;  // Success
                        }
                    }
                }

                // throw new ArgumentException("Invalid patch string: " + text[textPointer]);
                return false;
            } while (false);


            patch_t patch;

            parseInt(string_view_t { sp1, start1 }, patch.start1);
            if (length1 == 0) {
                patch.start1--;
                patch.length1 = 1;
            } else if (length1 == 1 && *lp1 == char_traits::cast('0')) {
                patch.length1 = 0;
            } else {
                patch.start1--;
                parseInt(string_view_t { lp1, length1 }, patch.length1);
            }

            parseInt(string_view_t { sp2, start2 }, patch.start2);
            if (length2 == 0) {
                patch.start2--;
                patch.length2 = 1;
            } else if (length2 == 1 && *lp2 == char_traits::cast('0')) {
                patch.length2 = 0;
            } else {
                patch.start2--;
                parseInt(string_view_t { lp2, length2 }, patch.length2);
            }

            char_t sign {};

            while (commons::nextLine(line, textline, start, len, char_traits::eol)) {
                t = line.data();
                l = line.length();

                if (l <= 0) {
                    // Blank line?  Whatever.
                    continue;
                }

                sign = t[0];
                if (sign == char_traits::cast('@')) {
                    // Start of next patch.
                    break;
                }

                line = line.substring(1);
                if (!pool.template percent_decode<char_traits>(line, true /*decode would change all "+" to " "*/)) {
                    return false;
                }
                if (sign == char_traits::cast('-')) {
                    // Deletion.
                    patch.diffs.push_back(diff_t(Operation::DELETE, line));
                } else if (sign == char_traits::cast('+')) {
                    // Insertion.
                    patch.diffs.push_back(diff_t(Operation::INSERT, line));
                } else if (sign == char_traits::cast(' ')) {
                    // Minor equality.
                    patch.diffs.push_back(diff_t(Operation::EQUAL, line));
                } else {
                    // WTF?
                    // throw new ArgumentException("Invalid patch mode '" + sign + "' in: " + line);
                    return false;
                }
            }


            patches.push_back(patch);
        }

        return true;
    }
};
}  // namespace dmp

#endif
