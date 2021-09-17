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


#ifndef DIFF_MATCH_PATCH_ALGORITHMS_DIFF_H
#define DIFF_MATCH_PATCH_ALGORITHMS_DIFF_H


#include "dmp/algorithms/dmp_algorithms_common.h"
#include "dmp/traits/dmp_encoding_sub_traits.h"
#include "dmp/types/dmp_diff.h"
#include "dmp/types/dmp_settings.h"
#include "dmp/utils/dmp_encoding.h"
#include "dmp/utils/dmp_stringpool_base.h"

namespace dmp {


/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */
template <class all_traits>
struct diff_match_patch_diff {
    using commons    = diff_match_patch_common<all_traits>;
    using settings_t = typename commons::settings_t;

    using clock_traits = typename all_traits::clock_traits;
    using clock_t      = typename clock_traits::type;

    using string_traits = typename all_traits::string_traits;

    using char_traits     = typename string_traits::char_traits;
    using string_view_t   = typename string_traits::string_view_t;
    using owning_string_t = typename string_traits::owning_string_t;
    using char_t          = typename string_traits::char_t;

    static constexpr const size_t npos = string_traits::npos;


    using container_traits = typename all_traits::container_traits;

    template <typename key, typename value>
    using encoding_hash_t = typename container_traits::template encoding_hash<key, value>;

    using string_pool_t = typename commons::string_pool_t;


    using Operation = typename commons::Operation;
    using diff_t    = typename commons::diff_t;
    using diffs_t   = typename commons::diffs_t;


    using encoding_helper_t = traits::diff_encoding_helper<diff_match_patch_diff<all_traits>>;

    using encoding_algorithm_diff_t = typename encoding_helper_t::algorithm_diff_t;
    using encoding_string_view_t    = typename encoding_algorithm_diff_t::string_view_t;
    using encoding_owning_string_t  = typename encoding_algorithm_diff_t::owning_string_t;
    using encoding_list_t           = typename container_traits::template encoding_list<string_view_t /*on purpose*/>;
    using encoding_char_t           = typename encoding_algorithm_diff_t::char_t;
    using encoding_string_pool_t    = typename encoding_algorithm_diff_t::string_pool_t;

    static constexpr const bool needsSubEncoder = encoding_helper_t::needsSubDiff;

    using encoding_diff_t  = typename encoding_algorithm_diff_t::diff_t;
    using encoding_diffs_t = typename encoding_algorithm_diff_t::diffs_t;

public:
    constexpr diff_match_patch_diff() noexcept = default;

    //  DIFF FUNCTIONS

    /**
     * Find the differences between two texts.
     * Run a faster, slightly less optimal diff.
     * This method allows the 'checklines' of diff_main() to be optional.
     * Most of the time checklines is wanted, so default to true.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @return List of Diff objects.
     */
public:
    inline static constexpr void diff_main(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1,
                                           string_view_t text2) noexcept {
        // Set a deadline by which time the diff must be complete.
        diff_main(settings, diffs, pool, text1, text2, true);
    }

    /**
     * Find the differences between two texts.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @param checklines Speedup flag.  If false, then don't run a
     *     line-level diff first to identify the changed areas.
     *     If true, then run a faster slightly less optimal diff.
     * @return List of Diff objects.
     */
public:
    inline static constexpr void diff_main(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1, string_view_t text2,
                                           bool checklines) noexcept {
        // Set a deadline by which time the diff must be complete.

        clock_t deadline;
        if (settings.Diff_Timeout > 0) {
            deadline = clock_t::now();
            deadline.addMilliseconds(static_cast<int64_t>(settings.Diff_Timeout * 1000.f));
        }

        diff_main(settings, diffs, pool, text1, text2, checklines, deadline);
    }


    /**
     * Find the differences between two texts.  Simplifies the problem by
     * stripping any common prefix or suffix off the texts before diffing.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @param checklines Speedup flag.  If false, then don't run a
     *     line-level diff first to identify the changed areas.
     *     If true, then run a faster slightly less optimal diff.
     * @param deadline Time when the diff should be complete by.  Used
     *     internally for recursive calls.  Users should set DiffTimeout
     *     instead.
     * @return List of Diff objects.
     */
public:
    inline static constexpr void diff_main(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1, string_view_t text2,
                                           bool checklines, clock_t deadline) noexcept {
        // Check for null inputs not needed since null can't be passed in C#.

        using namespace dmp::utils;

        diffs.clear();

        // Check for equality (speedup).
        if (text1 == text2) {
            if (text1.length() != 0) {
                diffs.push_back(diff_t(Operation::EQUAL, text1));
            }
            return;
        }

        // Trim off common prefix (speedup).
        auto          commonlength = commons::diff_commonPrefix(text1, text2);
        string_view_t commonprefix = text1.substring(0, commonlength);
        text1                      = text1.substring(commonlength);
        text2                      = text2.substring(commonlength);

        // Trim off common suffix (speedup).
        commonlength               = commons::diff_commonSuffix(text1, text2);
        string_view_t commonsuffix = text1.substring(text1.length() - commonlength);
        text1                      = text1.substring(0, text1.length() - commonlength);
        text2                      = text2.substring(0, text2.length() - commonlength);


        // Restore the prefix.

        // Compute the diff on the middle block.
        diff_compute(settings, diffs, pool, text1, text2, checklines, deadline);

        // Restore the prefix and suffix.
        if (commonprefix.length() != 0) {
            diffs.push_front(diff_t(Operation::EQUAL, commonprefix));
        }
        if (commonsuffix.length() != 0) {
            diffs.push_back(diff_t(Operation::EQUAL, commonsuffix));
        }

        diff_cleanupMerge(diffs, pool);
    }


protected:
    struct HalfMatchResult {
        string_view_t best_longtext_a;
        string_view_t best_longtext_b;
        string_view_t best_shorttext_a;
        string_view_t best_shorttext_b;
        string_view_t best_common;

        constexpr HalfMatchResult() = default;

        constexpr HalfMatchResult(const HalfMatchResult&) = default;
        constexpr HalfMatchResult(HalfMatchResult&&)      = default;

        constexpr HalfMatchResult& operator=(const HalfMatchResult&) = default;
        constexpr HalfMatchResult& operator=(HalfMatchResult&&) = default;

        inline constexpr HalfMatchResult(string_view_t _best_longtext_a, string_view_t _best_longtext_b, string_view_t _best_shorttext_a,
                                         string_view_t _best_shorttext_b, string_view_t _best_common) noexcept
            : best_longtext_a(_best_longtext_a)
            , best_longtext_b(_best_longtext_b)
            , best_shorttext_a(_best_shorttext_a)
            , best_shorttext_b(_best_shorttext_b)
            , best_common(_best_common) {}

        inline constexpr bool operator==(const HalfMatchResult& o) const noexcept {
            return best_longtext_a == o.best_longtext_a && best_longtext_b == o.best_longtext_b && best_shorttext_a == o.best_shorttext_a
                   && best_shorttext_b == o.best_shorttext_b && best_common == o.best_common;
        }

        inline constexpr bool operator!=(const HalfMatchResult& o) const noexcept { return !operator==(o); }

        /**
         * Does a substring of shorttext exist within longtext such that the
         * substring is at least half the length of longtext?
         * @param longtext Longer string.
         * @param shorttext Shorter string.
         * @param i Start index of quarter length substring within longtext.
         * @return Five element string array, containing the prefix of longtext, the
         *     suffix of longtext, the prefix of shorttext, the suffix of shorttext
         *     and the common middle.  Or null if there was no match.
         */
        inline constexpr bool diff_halfMatchI(string_view_t longtext, string_view_t shorttext, size_t i) noexcept {
            using namespace dmp::utils;

            // Start with a 1/4 length substring at position i as a seed.
            string_view_t seed = longtext.substring(i, longtext.length() / 4);
            size_t        j    = npos;
            while ((j == npos || j < shorttext.length()) && (j = shorttext.indexOf(seed, j + 1)) != npos) {
                size_t prefixLength = commons::diff_commonPrefix(longtext.substring(i), shorttext.substring(j));
                size_t suffixLength = commons::diff_commonSuffix(longtext.substring(0, i), shorttext.substring(0, j));
                if (best_common.length() < suffixLength + prefixLength) {
                    // @fix: optimization of original:
                    // best_common      = shorttext.substring(j - suffixLength, suffixLength) + shorttext.substring(j, prefixLength);
                    best_common      = shorttext.substring(j - suffixLength, suffixLength + prefixLength);
                    best_longtext_a  = longtext.substring(0, i - suffixLength);
                    best_longtext_b  = longtext.substring(i + prefixLength);
                    best_shorttext_a = shorttext.substring(0, j - suffixLength);
                    best_shorttext_b = shorttext.substring(j + prefixLength);
                }
            }
            if (best_common.length() * 2 >= longtext.length()) {
                return true;
            }

            return false;
        }
    };

    /**
     * Find the differences between two texts.  Assumes that the texts do not
     * have any common prefix or suffix.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @param checklines Speedup flag.  If false, then don't run a
     *     line-level diff first to identify the changed areas.
     *     If true, then run a faster slightly less optimal diff.
     * @param deadline Time when the diff should be complete by.
     * @return List of Diff objects.
     */
private:
    inline static constexpr void diff_compute(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1,
                                              string_view_t text2, bool checklines, clock_t deadline) noexcept {
        using namespace dmp::utils;


        if (text1.length() == 0) {
            // Just add some text (speedup).
            diffs.push_back(diff_t(Operation::INSERT, text2));
            return;
        }

        if (text2.length() == 0) {
            // Just delete some text (speedup).
            diffs.push_back(diff_t(Operation::DELETE, text1));
            return;
        }

        string_view_t longtext  = text1.length() > text2.length() ? text1 : text2;
        string_view_t shorttext = text1.length() > text2.length() ? text2 : text1;
        auto          i         = longtext.indexOf(shorttext);
        if (i != npos) {
            // Shorter text is inside the longer text (speedup).
            Operation op = (text1.length() > text2.length()) ? Operation::DELETE : Operation::INSERT;
            diffs.push_back(diff_t(op, longtext.substring(0, i)));
            diffs.push_back(diff_t(Operation::EQUAL, shorttext));
            diffs.push_back(diff_t(op, longtext.substring(i + shorttext.length())));
            return;
        }

        if (shorttext.length() == 1) {
            // Single character string.
            // After the previous speedup, the character can't be an equality.
            diffs.push_back(diff_t(Operation::DELETE, text1));
            diffs.push_back(diff_t(Operation::INSERT, text2));
            return;
        }


        // Check to see if the problem can be split in two.
        HalfMatchResult hm;
        if (diff_halfMatch(settings, hm, text1, text2)) {
            // A half-match was found, sort out the return data.
            // Send both pairs off for separate processing.
            // Merge the results.
            diff_main(settings, diffs, pool, hm.best_longtext_a, hm.best_shorttext_a, checklines, deadline);
            diffs.push_back(diff_t(Operation::EQUAL, hm.best_common));
            diffs_t diffs_b;
            diff_main(settings, diffs_b, pool, hm.best_longtext_b, hm.best_shorttext_b, checklines, deadline);
            diffs.addAll(diffs_b);
            return;
        }


        if (checklines && text1.length() > 100 && text2.length() > 100) {
            diff_lineMode(settings, diffs, pool, text1, text2, deadline);
            return;
        }

        diff_bisect(settings, diffs, pool, text1, text2, deadline);
    }


    /**
     * Do a quick line-level diff on both strings, then rediff the parts for
     * greater accuracy.
     * This speedup can produce non-minimal diffs.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @param deadline Time when the diff should be complete by.
     * @return List of Diff objects.
     */
private:
    inline static constexpr void diff_lineMode(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1,
                                               string_view_t text2, clock_t deadline) noexcept {
        using namespace dmp::utils;

        encoding_owning_string_t encodedStrings[2] {};

        encoding_list_t linearray;

        // Scan the text on a line-by-line basis first.
        diff_linesToChars(encodedStrings, text1, text2, linearray);

        encoding_string_view_t s1 { encodedStrings[0] };
        encoding_string_view_t s2 { encodedStrings[1] };


        using eoriginal_texts_t = typename encoding_string_pool_t::original_texts;
        eoriginal_texts_t      otexts[] { &s1, &s2 };
        encoding_string_pool_t stringpool(otexts);

        encoding_diffs_t eDiffs;
        encoding_algorithm_diff_t::diff_main(settings, eDiffs, stringpool, s1, s2, false, deadline);

        // Convert the diff back to original text.
        diff_charsToLines(diffs, pool, eDiffs, linearray);
        // Eliminate freak matches (e.g. blank lines)
        diff_cleanupSemantic(diffs, pool);

        // Rediff any replacement blocks, this time character-by-character.
        // Add a dummy entry at the end.
        diffs.push_back(diff_t(Operation::EQUAL, string_view_t {}));
        size_t        pointer      = 0;
        size_t        count_delete = 0;
        size_t        count_insert = 0;
        string_view_t text_delete {};
        string_view_t text_insert {};
        while (pointer < diffs.size()) {
            switch (diffs[pointer].operation) {
                case Operation::INSERT:
                    count_insert++;
                    pool.append(text_insert, diffs[pointer].text);
                    break;
                case Operation::DELETE:
                    count_delete++;
                    pool.append(text_delete, diffs[pointer].text);
                    break;
                case Operation::EQUAL:
                    // Upon reaching an equality, check for prior redundancies.
                    if (count_delete >= 1 && count_insert >= 1) {
                        // Delete the offending records and add the merged ones.
                        diffs_t subDiffs;
                        diff_main(settings, subDiffs, pool, text_delete, text_insert, false, deadline);

                        diffs.splice(pointer - count_delete - count_insert, count_delete + count_insert, subDiffs);
                        pointer = pointer - count_delete - count_insert;
                        pointer = pointer + subDiffs.size();
                    }
                    count_insert = 0;
                    count_delete = 0;
                    text_delete  = string_view_t {};
                    text_insert  = string_view_t {};
                    break;
            }
            pointer++;
        }
        diffs.pop_back();  // Remove the dummy entry at the end.
    }

    /**
     * Split two texts into a list of strings.  Reduce the texts to a string of
     * hashes where each Unicode character represents one line.
     * @param text1 First string.
     * @param text2 Second string.
     * @return Three element Object array, containing the encoded text1, the
     *     encoded text2 and the List of unique strings.  The zeroth element
     *     of the List of unique strings is intentionally blank.
     */
protected:
    inline static constexpr void diff_linesToChars(encoding_owning_string_t (&encodedStrings)[2], string_view_t text1, string_view_t text2,
                                                   encoding_list_t& lines) noexcept {
        using namespace dmp::utils;

        encodedStrings[0] = encoding_owning_string_t {};
        encodedStrings[1] = encoding_owning_string_t {};

        encoding_hash_t<string_view_t, encoding_char_t> lineHash;
        // e.g. linearray[4] == "Hello\n"
        // e.g. linehash.get("Hello\n") == 4

        // "\x00" is a valid character, but various debuggers don't like it.
        // So we'll insert a junk entry to avoid generating a null character.
        // lines.push_back(string_view_t {});

        // Allocate 2/3rds of the space for text1, the rest for text2.
        diff_linesToCharsMunge(encodedStrings[0], text1, lineHash, 40000);
        diff_linesToCharsMunge(encodedStrings[1], text2, lineHash, 65535);


        lines.resize(lineHash.size() + 1);
        for (auto& p : lineHash) {
            lines[p.second] = p.first;
        }
    }

    /**
     * Split a text into a list of strings.  Reduce the texts to a string of
     * hashes where each Unicode character represents one line.
     * @param text String to encode.
     * @param lineArray List of unique strings.
     * @param lineHash Map of strings to indices.
     * @param maxLines Maximum length of lineArray.
     * @return Encoded string.
     */
private:
    inline static constexpr void diff_linesToCharsMunge(encoding_owning_string_t& encoded_string, string_view_t text,
                                                        encoding_hash_t<string_view_t, encoding_char_t>& lineHash,
                                                        encoding_char_t                                  maxLines) noexcept {
        using namespace dmp::utils;

        size_t        lineStart = 0;
        size_t        lineEnd   = 0;
        string_view_t line {};

        // Walk the text, pulling out a substring for each line.
        // text.split(eol) would would temporarily double our memory footprint.
        // Modifying text would create many large strings to garbage collect.
        auto len(text.length());
        if (len <= 0) {
            return;
        }

        encoded_string.reserve(64);

        do {
            lineEnd = text.indexOf(char_traits::eol, lineStart);
            if (lineEnd == npos) {
                lineEnd = len - 1;
            }
            line = text.substring(lineStart, lineEnd + 1 - lineStart);

            encoding_char_t id(static_cast<encoding_char_t>(lineHash.size() + 1));
            if (id == maxLines) {
                // Bail out at 65535 because char 65536 == char 0.
                line    = text.substring(lineStart);
                lineEnd = text.length();
            }

            // we might need more than 256 chars here, so use at least a short
            encoded_string.push_back(lineHash.insertAndReturnValue(line, id));
            lineStart = lineEnd + 1;
        } while (lineEnd < len - 1);
    }

    /**
     * Rehydrate the text in a diff from a string of line hashes to real lines
     * of text.
     * @param diffs List of Diff objects.
     * @param lineArray List of unique strings.
     */
protected:
    inline static constexpr void diff_charsToLines(diffs_t& diffs, string_pool_t& pool, const encoding_diffs_t& eDiffs,
                                                   const encoding_list_t& lines) noexcept {
        using namespace dmp::utils;

        diffs.reserve(diffs.size() + eDiffs.size());

        for (auto& ed : eDiffs) {
            string_view_t text;

            for (auto& c : ed.text) {
                pool.append(text, lines[static_cast<size_t>(c)]);
            }

            diffs.push_back(diff_t(static_cast<Operation>(ed.operation), text));
        }
    }

protected:
    inline static constexpr bool diff_bisect(string_view_t text1, string_view_t text2, clock_t deadline, int& _x1, int& _y1) noexcept {
        using namespace dmp::utils;

        // Cache the text lengths to prevent multiple calls.
        int  text1_length = static_cast<int>(text1.length());
        int  text2_length = static_cast<int>(text2.length());
        auto d1(text1.data());
        auto d2(text2.data());
        int  max_d    = (text1_length + text2_length + 1) / 2;
        int  v_offset = max_d;
        int  v_length = 2 * max_d;

        using List = typename container_traits::template bisect_list<int>;
        List v1(static_cast<size_t>(v_length), -1);
        List v2(static_cast<size_t>(v_length), -1);

        v1[static_cast<size_t>(v_offset + 1)] = 0;
        v2[static_cast<size_t>(v_offset + 1)] = 0;
        int delta        = text1_length - text2_length;
        // If the total number of characters is odd, then the front path will
        // collide with the reverse path.
        bool front = (delta % 2 != 0);
        // Offsets for start and end of k loop.
        // Prevents mapping of space beyond the grid.
        int k1start = 0;
        int k1end   = 0;
        int k2start = 0;
        int k2end   = 0;
        for (int d = 0; d < max_d; d++) {
            // Bail out if deadline is reached.
            if (deadline.hitDeadline()) {
                break;
            }

            // Walk the front path one step.
            for (int k1 = -d + k1start; k1 <= d - k1end; k1 += 2) {
                int k1_offset = v_offset + k1;
                int x1 {};
                // @fix: parentheses
                if (k1 == -d || (k1 != d && v1[static_cast<size_t>(k1_offset - 1)] < v1[static_cast<size_t>(k1_offset + 1)])) {
                    x1 = v1[static_cast<size_t>(k1_offset + 1)];
                } else {
                    x1 = v1[static_cast<size_t>(k1_offset - 1)] + 1;
                }
                int y1 = x1 - k1;
                while (x1 < text1_length && y1 < text2_length && d1[x1] == d2[y1]) {
                    x1++;
                    y1++;
                }
                v1[static_cast<size_t>(k1_offset)] = x1;
                if (x1 > text1_length) {
                    // Ran off the right of the graph.
                    k1end += 2;
                } else if (y1 > text2_length) {
                    // Ran off the bottom of the graph.
                    k1start += 2;
                } else if (front) {
                    int k2_offset = v_offset + delta - k1;
                    if (k2_offset >= 0 && k2_offset < v_length && v2[static_cast<size_t>(k2_offset)] != -1) {
                        // Mirror x2 onto top-left coordinate system.
                        int x2 = text1_length - v2[static_cast<size_t>(k2_offset)];
                        if (x1 >= x2) {
                            // Overlap detected.

                            _x1 = x1;
                            _y1 = y1;
                            return true;
                        }
                    }
                }
            }

            // Walk the reverse path one step.
            for (int k2 = -d + k2start; k2 <= d - k2end; k2 += 2) {
                int k2_offset = v_offset + k2;
                int x2 {};
                // @fix: parentheses
                if (k2 == -d || (k2 != d && v2[static_cast<size_t>(k2_offset - 1)] < v2[static_cast<size_t>(k2_offset + 1)])) {
                    x2 = v2[static_cast<size_t>(k2_offset + 1)];
                } else {
                    x2 = v2[static_cast<size_t>(k2_offset - 1)] + 1;
                }
                int y2 = x2 - k2;
                while (x2 < text1_length && y2 < text2_length && d1[text1_length - x2 - 1] == d2[text2_length - y2 - 1]) {
                    x2++;
                    y2++;
                }
                v2[static_cast<size_t>(k2_offset)] = x2;
                if (x2 > text1_length) {
                    // Ran off the left of the graph.
                    k2end += 2;
                } else if (y2 > text2_length) {
                    // Ran off the top of the graph.
                    k2start += 2;
                } else if (!front) {
                    int k1_offset = v_offset + delta - k2;
                    if (k1_offset >= 0 && k1_offset < v_length && v1[static_cast<size_t>(k1_offset)] != -1) {
                        int x1 = v1[static_cast<size_t>(k1_offset)];
                        // Mirror x2 onto top-left coordinate system.
                        x2 = text1_length - v2[static_cast<size_t>(k2_offset)];
                        if (x1 >= x2) {
                            // Overlap detected.

                            _x1 = x1;
                            _y1 = v_offset + x1 - k1_offset;
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    inline static constexpr void diff_bisect(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1,
                                             string_view_t text2, clock_t deadline) noexcept {
        using namespace dmp::utils;

        int x1 {};
        int y1 {};
        if (diff_bisect(text1, text2, deadline, x1, y1)) {
            // this separation into a separate function reduces required stack size
            diff_bisectSplit(settings, diffs, pool, text1, text2, static_cast<size_t>(x1), static_cast<size_t>(y1), deadline);
            return;
        }


        // Diff took too long and hit the deadline or
        // number of diffs equals number of characters, no commonality at all.
        diffs.push_back(diff_t(Operation::DELETE, text1));
        diffs.push_back(diff_t(Operation::INSERT, text2));
        return;
    }

    /**
     * Given the location of the 'middle snake', split the diff in two parts
     * and recurse.
     * @param text1 Old string to be diffed.
     * @param text2 New string to be diffed.
     * @param x Index of split point in text1.
     * @param y Index of split point in text2.
     * @param deadline Time at which to bail if not yet complete.
     * @return LinkedList of Diff objects.
     */
private:
    inline static constexpr void diff_bisectSplit(const settings_t& settings, diffs_t& diffs, string_pool_t& pool, string_view_t text1,
                                                  string_view_t text2, size_t x, size_t y, clock_t deadline) noexcept {
        string_view_t text1a = text1.substring(0, x);
        string_view_t text2a = text2.substring(0, y);
        string_view_t text1b = text1.substring(x);
        string_view_t text2b = text2.substring(y);

        // Compute both diffs serially.
        diff_main(settings, diffs, pool, text1a, text2a, false, deadline);
        diffs_t diffs_b;
        diff_main(settings, diffs_b, pool, text1b, text2b, false, deadline);
        diffs.addAll(diffs_b);
    }


    /**
     * Do the two texts share a substring which is at least half the length of
     * the longer text?
     * This speedup can produce non-minimal diffs.
     * @param text1 First string.
     * @param text2 Second string.
     * @return Five element String array, containing the prefix of text1, the
     *     suffix of text1, the prefix of text2, the suffix of text2 and the
     *     common middle.  Or null if there was no match.
     */
protected:
    inline static constexpr bool diff_halfMatch(const settings_t& settings, HalfMatchResult& hm, string_view_t text1, string_view_t text2) noexcept {
        using namespace dmp::utils;

        if (settings.Diff_Timeout <= 0) {
            // Don't risk returning a non-optimal diff if we have unlimited time.
            return false;
        }

        using namespace dmp::utils;

        string_view_t longtext  = text1.length() > text2.length() ? text1 : text2;
        string_view_t shorttext = text1.length() > text2.length() ? text2 : text1;
        if (longtext.length() < 4 || shorttext.length() * 2 < longtext.length()) {
            return false;  // Pointless.
        }

        // First check if the second quarter is the seed for a half-match.
        bool res1(hm.diff_halfMatchI(longtext, shorttext, (longtext.length() + 3) / 4));

        // Check again based on the third quarter.
        HalfMatchResult hm2;
        bool            res2(hm2.diff_halfMatchI(longtext, shorttext, (longtext.length() + 1) / 2));

        if (!res1 && !res2) {
            return false;
        } else if (!res2) {
            // hm = hm1;
        } else if (!res1) {
            hm = hm2;
        } else {
            // Both matched.  Select the longest.
            if (hm.best_common.length() <= hm2.best_common.length()) {
                hm = hm2;
            }
        }

        // A half-match was found, sort out the return data.
        if (text1.length() > text2.length()) {
            return true;
            // return new string[]{hm[0], hm[1], hm[2], hm[3], hm[4]};
        } else {
            // { best_longtext_a, best_longtext_b, best_shorttext_a, best_shorttext_b, best_common };
            utils::swap(hm.best_longtext_a, hm.best_shorttext_a);
            utils::swap(hm.best_longtext_b, hm.best_shorttext_b);
            // return new string[] { hm[2], hm[3], hm[0], hm[1], hm[4] };
            return true;
        }
    }

    /**
     * Reduce the number of edits by eliminating semantically trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    inline static constexpr void diff_cleanupSemantic(diffs_t& diffs, string_pool_t& pool) noexcept {
        if (diffs.size() <= 0) {
            return;
        }

        using namespace dmp::utils;

        bool changes = false;
        // Stack of indices where equalities are found.
        using List = typename container_traits::template equalities_list<int>;
        List equalities;
        equalities.reserve(64);
        // Always equal to equalities[equalitiesLength-1][1]
        string_view_t lastEquality {};
        int           pointer = 0;  // Index of current position.
        // Number of characters that changed prior to the equality.
        size_t length_insertions1 = 0;
        size_t length_deletions1  = 0;
        // Number of characters that changed after the equality.
        size_t length_insertions2 = 0;
        size_t length_deletions2  = 0;
        while (static_cast<size_t>(pointer) < diffs.size()) {
            if (diffs[static_cast<size_t>(pointer)].operation == Operation::EQUAL) {  // Equality found.
                equalities.push_back(pointer);
                length_insertions1 = length_insertions2;
                length_deletions1  = length_deletions2;
                length_insertions2 = 0;
                length_deletions2  = 0;
                lastEquality       = diffs[static_cast<size_t>(pointer)].text;
            } else {  // an insertion or deletion
                if (diffs[static_cast<size_t>(pointer)].operation == Operation::INSERT) {
                    length_insertions2 += diffs[static_cast<size_t>(pointer)].text.length();
                } else {
                    length_deletions2 += diffs[static_cast<size_t>(pointer)].text.length();
                }
                // Eliminate an equality that is smaller or equal to the edits on both
                // sides of it.
                if (lastEquality.length() > 0 && (lastEquality.length() <= max(length_insertions1, length_deletions1))
                    && (lastEquality.length() <= max(length_insertions2, length_deletions2))) {
                    // Duplicate record.
                    diffs.splice(static_cast<size_t>(equalities.back()), 0, diff_t(Operation::DELETE, lastEquality));
                    // Change second copy to insert.
                    diffs[static_cast<size_t>(equalities.back() + 1)].operation = Operation::INSERT;
                    // Throw away the equality we just deleted.
                    equalities.pop_back();
                    if (equalities.size() > 0) {
                        equalities.pop_back();
                    }
                    pointer            = equalities.size() > 0 ? equalities.back() : -1;
                    length_insertions1 = 0;  // Reset the counters.
                    length_deletions1  = 0;
                    length_insertions2 = 0;
                    length_deletions2  = 0;
                    lastEquality       = string_view_t {};
                    changes            = true;
                }
            }
            pointer++;
        }

        // Normalize the diff.
        if (changes) {
            diff_cleanupMerge(diffs, pool);
        }
        diff_cleanupSemanticLossless(diffs, pool);

        // Find any overlaps between deletions and insertions.
        // e.g: <del>abcxxx</del><ins>xxxdef</ins>
        //   -> <del>abc</del>xxx<ins>def</ins>
        // e.g: <del>xxxabc</del><ins>defxxx</ins>
        //   -> <ins>def</ins>xxx<del>abc</del>
        // Only extract an overlap if it is as big as the edit ahead or behind it.
        pointer = 1;
        while (static_cast<size_t>(pointer) < diffs.size()) {
            if (diffs[static_cast<size_t>(pointer - 1)].operation == Operation::DELETE && diffs[static_cast<size_t>(pointer)].operation == Operation::INSERT) {
                string_view_t deletion        = diffs[static_cast<size_t>(pointer - 1)].text;
                string_view_t insertion       = diffs[static_cast<size_t>(pointer)].text;
                auto          overlap_length1 = commons::diff_commonOverlap(deletion, insertion);
                auto          overlap_length2 = commons::diff_commonOverlap(insertion, deletion);
                if (overlap_length1 >= overlap_length2) {
                    if (overlap_length1 >= deletion.length() / 2.0 || overlap_length1 >= insertion.length() / 2.0) {
                        // Overlap found.
                        // Insert an equality and trim the surrounding edits.
                        diffs.splice(pointer < 0 ? 0 : static_cast<size_t>(pointer), 0,
                                     diff_t(Operation::EQUAL, insertion.substring(0, overlap_length1)));
                        diffs[static_cast<size_t>(pointer - 1)].text = deletion.substring(0, deletion.length() - overlap_length1);
                        diffs[static_cast<size_t>(pointer + 1)].text = insertion.substring(overlap_length1);
                        pointer++;
                    }
                } else {
                    if (overlap_length2 >= deletion.length() / 2.0 || overlap_length2 >= insertion.length() / 2.0) {
                        // Reverse overlap found.
                        // Insert an equality and swap and trim the surrounding edits.
                        diffs.splice(pointer < 0 ? 0 : static_cast<size_t>(pointer), 0,
                                     diff_t(Operation::EQUAL, deletion.substring(0, overlap_length2)));
                        diffs[static_cast<size_t>(pointer - 1)].operation = Operation::INSERT;
                        diffs[static_cast<size_t>(pointer - 1)].text      = insertion.substring(0, insertion.length() - overlap_length2);
                        diffs[static_cast<size_t>(pointer + 1)].operation = Operation::DELETE;
                        diffs[static_cast<size_t>(pointer + 1)].text      = deletion.substring(overlap_length2);
                        pointer++;
                    }
                }
                pointer++;
            }
            pointer++;
        }
    }


    /**
     * Look for single edits surrounded on both sides by equalities
     * which can be shifted sideways to align the edit to a word boundary.
     * e.g: The c<ins>at c</ins>ame. -> The <ins>cat </ins>came.
     * @param diffs List of Diff objects.
     */
public:
    inline static constexpr void diff_cleanupSemanticLossless(diffs_t& diffs, string_pool_t& pool) noexcept {
        if (diffs.size() <= 0) {
            return;
        }

        size_t pointer = 1;
        // Intentionally ignore the first and last element (don't need checking).
        while (pointer < diffs.size() - 1) {
            if (diffs[pointer - 1].operation == Operation::EQUAL && diffs[pointer + 1].operation == Operation::EQUAL) {
                // This is a single edit surrounded by equalities.
                string_view_t equality1 = diffs[pointer - 1].text;
                string_view_t edit      = diffs[pointer].text;
                string_view_t equality2 = diffs[pointer + 1].text;

                // First, shift the edit as far left as possible.
                auto commonOffset = commons::diff_commonSuffix(equality1, edit);
                if (commonOffset > 0) {
                    string_view_t commonString = edit.substring(edit.length() - commonOffset);
                    equality1                  = equality1.substring(0, equality1.length() - commonOffset);
                    edit                       = pool.appended(commonString, edit.substring(0, edit.length() - commonOffset));
                    equality2                  = pool.appended(commonString, equality2);
                }

                // Second, step character by character right,
                // looking for the best fit.
                string_view_t bestEquality1 = equality1;
                string_view_t bestEdit      = edit;
                string_view_t bestEquality2 = equality2;
                auto          bestScore     = diff_cleanupSemanticScore(equality1, edit) + diff_cleanupSemanticScore(edit, equality2);
                while (edit.length() != 0 && equality2.length() != 0 && *edit.begin() == *equality2.begin()) {
                    pool.append(equality1, edit.substring(0, 1));
                    edit       = pool.appended(edit.substring(1), equality2.substring(0, 1));
                    equality2  = equality2.substring(1);
                    auto score = diff_cleanupSemanticScore(equality1, edit) + diff_cleanupSemanticScore(edit, equality2);
                    // The >= encourages trailing rather than leading whitespace on
                    // edits.
                    if (score >= bestScore) {
                        bestScore     = score;
                        bestEquality1 = equality1;
                        bestEdit      = edit;
                        bestEquality2 = equality2;
                    }
                }

                if (diffs[pointer - 1].text != bestEquality1) {
                    // We have an improvement, save it back to the diff.
                    if (bestEquality1.length() != 0) {
                        diffs[pointer - 1].text = bestEquality1;
                    } else {
                        diffs.splice(pointer - 1, 1);
                        pointer--;
                    }
                    diffs[pointer].text = bestEdit;
                    if (bestEquality2.length() != 0) {
                        diffs[pointer + 1].text = bestEquality2;
                    } else {
                        diffs.splice(pointer + 1, 1);
                        pointer--;
                    }
                }
            }
            pointer++;
        }
    }

    /**
     * Given two strings, compute a score representing whether the internal
     * boundary falls on logical boundaries.
     * Scores range from 6 (best) to 0 (worst).
     * @param one First string.
     * @param two Second string.
     * @return The score.
     */
private:
    inline static constexpr int diff_cleanupSemanticScore(string_view_t one, string_view_t two) noexcept {
        if (one.length() == 0 || two.length() == 0) {
            // Edges are the best.
            return 6;
        }

        // Each port of this function behaves slightly differently due to
        // subtle differences in each language's definition of things like
        // 'whitespace'.  Since this function's purpose is largely cosmetic,
        // the choice has been made to use each language's native features
        // rather than force total conformity.
        auto char1            = *(one.begin() + one.length() - 1);
        auto char2            = *two.begin();
        bool nonAlphaNumeric1 = !char_traits::isAlphaNum(char1);
        bool nonAlphaNumeric2 = !char_traits::isAlphaNum(char2);
        bool whitespace1      = nonAlphaNumeric1 && char_traits::isSpace(char1);
        bool whitespace2      = nonAlphaNumeric2 && char_traits::isSpace(char2);
        bool lineBreak1       = whitespace1 && char_traits::isControl(char1);
        bool lineBreak2       = whitespace2 && char_traits::isControl(char2);

        bool blankLine1 = false;
        if (lineBreak1) {
            auto d1(one.data());
            int  p1n(static_cast<int>(one.length()));
            auto p1(d1 + p1n - 1);

            auto p2(d1);
            if (*p1 == char_traits::eol) {
                p1n--;
                p1 = d1 + p1n - 1;
                if (p1 != p2) {
                    if (*p1 == char_traits::ret) {
                        p1n--;
                        p1 = d1 + p1n - 1;
                    }
                    blankLine1 = p1 != p2 && *p1 == char_traits::eol;
                }
            }
        }
        bool blankLine2 = false;
        if (lineBreak2) {
            auto p1(two.end());
            auto p2(two.begin());
            if (*p2 == char_traits::ret) {
                ++p2;
            }
            if (p2 != p1 && *p2 == char_traits::eol) {
                if (++p2 != p1 && *p2 == char_traits::ret) {
                    ++p2;
                }
                if (p2 != p1 && *p2 == char_traits::eol) {
                    blankLine2 = true;
                }
            }
        }

        if (blankLine1 || blankLine2) {
            // Five points for blank lines.
            return 5;
        } else if (lineBreak1 || lineBreak2) {
            // Four points for line breaks.
            return 4;
        } else if (nonAlphaNumeric1 && !whitespace1 && whitespace2) {
            // Three points for end of sentences.
            return 3;
        } else if (whitespace1 || whitespace2) {
            // Two points for whitespace.
            return 2;
        } else if (nonAlphaNumeric1 || nonAlphaNumeric2) {
            // One point for non-alphanumeric.
            return 1;
        }
        return 0;
    }


    // Define some regex patterns for matching boundaries.
    // Regex BLANKLINEEND = new Regex("\\n\\r?\\n\\Z");
    // Regex BLANKLINESTART = new Regex("\\A\\r?\\n\\r?\\n");

    /**
     * Reduce the number of edits by eliminating operationally trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    inline static constexpr void diff_cleanupEfficiency(const settings_t& settings, diffs_t& diffs, string_pool_t& pool) noexcept {
        if (diffs.size() <= 0) {
            return;
        }

        bool changes = false;
        // Stack of indices where equalities are found.
        using List = typename container_traits::template equalities_list<int>;
        List equalities;
        equalities.reserve(64);
        // Always equal to equalities[equalitiesLength-1][1]
        string_view_t lastEquality {};
        int           pointer = 0;  // Index of current position.
        // Is there an insertion operation before the last equality.
        bool pre_ins = false;
        // Is there a deletion operation before the last equality.
        bool pre_del = false;
        // Is there an insertion operation after the last equality.
        bool post_ins = false;
        // Is there a deletion operation after the last equality.
        bool post_del = false;
        while (static_cast<size_t>(pointer) < diffs.size()) {
            if (diffs[static_cast<size_t>(pointer)].operation == Operation::EQUAL) {  // Equality found.
                if (diffs[static_cast<size_t>(pointer)].text.length() < static_cast<size_t>(settings.Diff_EditCost) && (post_ins || post_del)) {
                    // Candidate found.
                    equalities.push_back(pointer);
                    pre_ins      = post_ins;
                    pre_del      = post_del;
                    lastEquality = diffs[static_cast<size_t>(pointer)].text;
                } else {
                    // Not a candidate, and can never become one.
                    equalities.clear();
                    lastEquality = string_view_t {};
                }
                post_ins = post_del = false;
            } else {  // An insertion or deletion.
                if (diffs[static_cast<size_t>(pointer)].operation == Operation::DELETE) {
                    post_del = true;
                } else {
                    post_ins = true;
                }
                /*
                 * Five types to be split:
                 * <ins>A</ins><del>B</del>XY<ins>C</ins><del>D</del>
                 * <ins>A</ins>X<ins>C</ins><del>D</del>
                 * <ins>A</ins><del>B</del>X<ins>C</ins>
                 * <ins>A</del>X<ins>C</ins><del>D</del>
                 * <ins>A</ins><del>B</del>X<del>C</del>
                 */
                if ((lastEquality.length() != 0)
                    && ((pre_ins && pre_del && post_ins && post_del)
                        || ((lastEquality.length() < static_cast<size_t>(settings.Diff_EditCost / 2))
                            && ((pre_ins ? 1 : 0) + (pre_del ? 1 : 0) + (post_ins ? 1 : 0) + (post_del ? 1 : 0)) == 3))) {
                    // Duplicate record.
                    diffs.splice(static_cast<size_t>(equalities.back()), 0, diff_t(Operation::DELETE, lastEquality));
                    // Change second copy to insert.
                    diffs[static_cast<size_t>(equalities.back() + 1)].operation = Operation::INSERT;
                    equalities.pop_back();  // Throw away the equality we just deleted.
                    lastEquality = string_view_t {};
                    if (pre_ins && pre_del) {
                        // No changes made which could affect previous entry, keep going.
                        post_ins = post_del = true;
                        equalities.clear();
                    } else {
                        if (equalities.size() > 0) {
                            equalities.pop_back();
                        }

                        pointer  = equalities.size() > 0 ? equalities.back() : -1;
                        post_ins = post_del = false;
                    }
                    changes = true;
                }
            }
            pointer++;
        }

        if (changes) {
            diff_cleanupMerge(diffs, pool);
        }
    }


    /**
     * Reorder and merge like edit sections.  Merge equalities.
     * Any edit section can move as long as it doesn't cross an equality.
     * @param diffs List of Diff objects.
     */
public:
    inline static constexpr void diff_cleanupMerge(diffs_t& diffs, string_pool_t& pool) noexcept {
        if (diffs.size() <= 0) {
            return;
        }

        // Add a dummy entry at the end.
        diffs.push_back(diff_t(Operation::EQUAL, string_view_t {}));

        size_t        pointer      = 0;
        size_t        count_delete = 0;
        size_t        count_insert = 0;
        string_view_t text_delete  = {};
        string_view_t text_insert  = {};
        size_t        commonlength = 0;
        while (pointer < diffs.size()) {
            switch (diffs[pointer].operation) {
                case Operation::INSERT:
                    count_insert++;
                    pool.append(text_insert, diffs[pointer].text);
                    pointer++;
                    break;
                case Operation::DELETE:
                    count_delete++;
                    pool.append(text_delete, diffs[pointer].text);
                    pointer++;
                    break;
                case Operation::EQUAL:
                    // Upon reaching an equality, check for prior redundancies.
                    if (count_delete + count_insert > 1) {
                        if (count_delete != 0 && count_insert != 0) {
                            // Factor out any common prefixies.
                            commonlength = commons::diff_commonPrefix(text_insert, text_delete);
                            if (commonlength != 0) {
                                if ((pointer - count_delete - count_insert) > 0
                                    && diffs[pointer - count_delete - count_insert - 1].operation == Operation::EQUAL) {
                                    pool.append(diffs[pointer - count_delete - count_insert - 1].text, text_insert.substring(0, commonlength));
                                } else {
                                    diffs.push_front(diff_t(Operation::EQUAL, text_insert.substring(0, commonlength)));
                                    pointer++;
                                }
                                text_insert = text_insert.substring(commonlength);
                                text_delete = text_delete.substring(commonlength);
                            }
                            // Factor out any common suffixies.
                            commonlength = commons::diff_commonSuffix(text_insert, text_delete);
                            if (commonlength != 0) {
                                diffs[pointer].text = pool.appended(text_insert.substring(text_insert.length() - commonlength), diffs[pointer].text);
                                text_insert         = text_insert.substring(0, text_insert.length() - commonlength);
                                text_delete         = text_delete.substring(0, text_delete.length() - commonlength);
                            }
                        }
                        // Delete the offending records and add the merged ones.
                        pointer -= count_delete + count_insert;

                        if (text_delete.length() != 0 && text_insert.length() != 0) {
                            diffs.splice(pointer, count_delete + count_insert, diff_t(Operation::DELETE, text_delete),
                                         diff_t(Operation::INSERT, text_insert));
                            pointer += 2;

                        } else if (text_delete.length() != 0) {
                            diffs.splice(pointer, count_delete + count_insert, diff_t(Operation::DELETE, text_delete));
                            pointer++;

                        } else if (text_insert.length() != 0) {
                            diffs.splice(pointer, count_delete + count_insert, diff_t(Operation::INSERT, text_insert));
                            pointer++;

                        } else {
                            diffs.splice(pointer, count_delete + count_insert);
                        }
                        pointer++;
                    } else if (pointer != 0 && diffs[pointer - 1].operation == Operation::EQUAL) {
                        // Merge this equality with the previous one.
                        pool.append(diffs[pointer - 1].text, diffs[pointer].text);
                        diffs.splice(pointer, 1);
                    } else {
                        pointer++;
                    }
                    count_insert = 0;
                    count_delete = 0;
                    text_delete  = string_view_t {};
                    text_insert  = string_view_t {};
                    break;
            }
        }
        if (diffs[diffs.size() - 1].text.length() == 0) {
            diffs.pop_back();  // Remove the dummy entry at the end.
        }

        // Second pass: look for single edits surrounded on both sides by
        // equalities which can be shifted sideways to eliminate an equality.
        // e.g: A<ins>BA</ins>C -> <ins>AB</ins>AC
        bool changes = false;
        pointer      = 1;
        // Intentionally ignore the first and last element (don't need checking).
        while (pointer < (diffs.size() - 1)) {
            if (diffs[pointer - 1].operation == Operation::EQUAL && diffs[pointer + 1].operation == Operation::EQUAL) {
                // This is a single edit surrounded by equalities.
                if (diffs[pointer].text.endsWith(diffs[pointer - 1].text)) {
                    // Shift the edit over the previous equality.
                    diffs[pointer].text = pool.appended(
                        diffs[pointer - 1].text, diffs[pointer].text.substring(0, diffs[pointer].text.length() - diffs[pointer - 1].text.length()));
                    diffs[pointer + 1].text = pool.appended(diffs[pointer - 1].text, diffs[pointer + 1].text);
                    diffs.splice(pointer - 1, 1);
                    changes = true;
                } else if (diffs[pointer].text.startsWith(diffs[pointer + 1].text)) {
                    // Shift the edit over the next equality.
                    pool.append(diffs[pointer - 1].text, diffs[pointer + 1].text);
                    diffs[pointer].text = pool.appended(diffs[pointer].text.substring(diffs[pointer + 1].text.length()), diffs[pointer + 1].text);
                    diffs.splice(pointer + 1, 1);
                    changes = true;
                }
            }
            pointer++;
        }
        // If shifts were made, the diff needs reordering and another shift sweep.
        if (changes) {
            diff_cleanupMerge(diffs, pool);
        }
    }


    /**
     * loc is a location in text1, compute and return the equivalent location in
     * text2.
     * e.g. "The cat" vs "The big cat", 1->1, 5->8
     * @param diffs List of Diff objects.
     * @param loc Location within text1.
     * @return Location within text2.
     */
public:
    inline static constexpr size_t diff_xIndex(const diffs_t& diffs, size_t loc) noexcept {
        size_t        chars1      = 0;
        size_t        chars2      = 0;
        size_t        last_chars1 = 0;
        size_t        last_chars2 = 0;
        const diff_t* lastDiff    = nullptr;
        for (auto& d : diffs) {
            if (d.operation != Operation::INSERT) {
                // Equality or deletion.
                chars1 += d.text.length();
            }
            if (d.operation != Operation::DELETE) {
                // Equality or insertion.
                chars2 += d.text.length();
            }
            if (chars1 > loc) {
                // Overshot the location.
                lastDiff = &d;
                break;
            }
            last_chars1 = chars1;
            last_chars2 = chars2;
        }
        if (lastDiff && lastDiff->operation == Operation::DELETE) {
            // The location was deleted.
            return last_chars2;
        }
        // Add the remaining character length.
        return last_chars2 + (loc - last_chars1);
    }

    /**
     * Given the original text1, and an encoded string which describes the
     * operations required to transform text1 into text2, compute the full diff.
     * @param text1 Source string for the diff.
     * @param delta Delta text.
     * @return Array of Diff objects or null if invalid.
     * @throws ArgumentException If invalid input.
     */
public:
    inline static constexpr bool diff_fromDelta(diffs_t& diffs, string_pool_t& pool, string_view_t text1, string_view_t delta) noexcept {
        diffs.clear();

        using namespace dmp::utils;

        int           n  = 0;
        size_t        tl = text1.length();
        string_view_t text;


        size_t        pointer = 0;  // Cursor in text1
        size_t        start   = 0;
        size_t        len     = delta.length();
        string_view_t token;
        while (commons::nextLine(token, delta, start, len, char_traits::tab)) {

            if (token.length() == 0) {
                // Blank tokens are ok (from a trailing \t).
                continue;
            }
            // Each token begins with a one character parameter which specifies the
            // operation of this token (delete, insert, equality).
            string_view_t param = token.substring(1);
            switch (*token.begin()) {
                case '+':
                    if (!pool.template percent_decode<char_traits>(param, true /*decode would change all "+" to " "*/)) {
                        return false;
                    }
                    //} catch (UnsupportedEncodingException e) {
                    //  // Not likely on modern system.
                    //  throw new Error("This system does not support UTF-8.", e);
                    //} catch (IllegalArgumentException e) {
                    //  // Malformed URI sequence.
                    //  throw new IllegalArgumentException(
                    //      "Illegal escape in diff_fromDelta: " + param, e);
                    //}
                    diffs.push_back(diff_t(Operation::INSERT, param));
                    break;
                case '-':
                    [[fallthrough]];
                case '=':
                    if (!parseInt(param, n)) {
                        // throw new ArgumentException("Invalid number in diff_fromDelta: " + param, e);
                        return false;
                    }
                    if (n < 0) {
                        // throw new ArgumentException("Negative number in diff_fromDelta: " + param);
                        return false;
                    }

                    if (pointer >= tl) {
                        // throw owning_string_t("Delta length (" + pointer + ") larger than source text length (" + text1.Length + ").", e);
                        return false;
                    }
                    text = text1.substring(pointer, static_cast<size_t>(n));
                    pointer += static_cast<size_t>(n);

                    if (*token.begin() == char_traits::cast('=')) {
                        diffs.push_back(diff_t(Operation::EQUAL, text));
                    } else {
                        diffs.push_back(diff_t(Operation::DELETE, text));
                    }
                    break;

                default:
                    // Anything else is an error.
                    // throw new ArgumentException("Invalid diff operation in diff_fromDelta: " + token[0]);
                    return false;
            }
        }

        if (pointer != tl) {
            // throw new ArgumentException("Delta length (" + pointer + ") smaller than source text length (" + text1.Length + ").");
            return false;
        }

        return true;
    }
};
}  // namespace dmp

#endif
