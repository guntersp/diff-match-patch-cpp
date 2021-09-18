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


#ifndef DIFF_MATCH_PATCH_ALGORITHMS_MATCH_H
#define DIFF_MATCH_PATCH_ALGORITHMS_MATCH_H


#include "dmp/algorithms/dmp_algorithms_common.h"
#include "dmp/types/dmp_settings.h"
#include "dmp/utils/dmp_stringpool_base.h"

namespace dmp {


/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */
template <class all_traits>
struct diff_match_patch_match {
    using commons    = diff_match_patch_common<all_traits>;
    using settings_t = typename commons::settings_t;

    using string_traits = typename all_traits::string_traits;

    using char_traits     = typename string_traits::char_traits;
    using char_t          = typename char_traits::char_t;
    using string_view_t   = typename string_traits::string_view_t;
    using owning_string_t = typename string_traits::owning_string_t;

    static constexpr const size_t npos = string_traits::npos;


    using container_traits = typename all_traits::container_traits;

    using alphabet_hash_t = typename container_traits::template alphabet_hash<char_t, size_t>;

public:
    constexpr diff_match_patch_match() noexcept = default;

    //  MATCH FUNCTIONS


    /**
     * Locate the best instance of 'pattern' in 'text' near 'loc'.
     * Returns -1 if no match found.
     * @param text The text to search.
     * @param pattern The pattern to search for.
     * @param loc The location to search around.
     * @return Best match index or -1.
     */
public:
    inline static constexpr size_t match_main(const settings_t& settings, string_view_t text, string_view_t pattern, size_t loc) noexcept {
        // Check for null inputs not needed since null can't be passed in C#.

        using namespace dmp::utils;

        auto tl(text.length());

        loc = max(size_t(0), min(loc, tl));
        if (text == pattern) {
            // Shortcut (potentially not guaranteed by the algorithm)
            return 0;
        } else if (tl == 0) {
            // Nothing to match.
            return npos;
        } else if (loc + pattern.length() <= tl && text.substring(loc, pattern.length()) == pattern) {
            // Perfect match at the perfect spot!  (Includes case of null pattern)
            return loc;
        } else {
            // Do a fuzzy compare.
            return match_bitap(settings, text, pattern, loc);
        }
    }

    /**
     * Locate the best instance of 'pattern' in 'text' near 'loc' using the
     * Bitap algorithm.  Returns -1 if no match found.
     * @param text The text to search.
     * @param pattern The pattern to search for.
     * @param loc The location to search around.
     * @return Best match index or -1.
     */
protected:
    inline static constexpr size_t match_bitap(const settings_t& settings, string_view_t text, string_view_t pattern, size_t loc) noexcept {
        // assert (Match_MaxBits == 0 || pattern.length() <= Match_MaxBits)
        //    : "Pattern too long for this application.";

        using namespace dmp::utils;

        using match_temp_list = typename container_traits::template match_temp_list<size_t>;

        // Initialise the alphabet.
        alphabet_hash_t s;
        match_alphabet(pattern, s);

        auto pl(pattern.length());
        auto tl(text.length());

        // Highest score beyond which we give up.
        double score_threshold = static_cast<double>(settings.Match_Threshold);
        // Is there a nearby exact match? (speedup)
        size_t best_loc = text.indexOf(pattern, loc);
        if (best_loc != npos) {
            score_threshold = min(match_bitapScore(settings, size_t(0), best_loc, loc, pattern), score_threshold);
            // What about in the other direction? (speedup)
            best_loc = text.lastIndexOf(pattern, min(loc + pl, tl));
            if (best_loc != npos) {
                score_threshold = min(match_bitapScore(settings, size_t(0), best_loc, loc, pattern), score_threshold);
            }
        }

        // Initialise the bit arrays.
        int matchmask = 1 << (pl - 1);
        best_loc      = npos;

        int bin_min {};
        int bin_mid {};
        int bin_max = static_cast<int>(pl + tl);
        // Empty initialization added to appease C# compiler.
        match_temp_list last_rd;
        match_temp_list rd;

        //auto pdata(pattern.data());
        auto tdata(text.data());
        for (size_t d = 0; d < pl; d++) {
            // Scan for the best match; each iteration allows for one more error.
            // Run a binary search to determine how far from 'loc' we can stray at
            // this error level.
            bin_min = 0;
            bin_mid = bin_max;
            while (bin_min < bin_mid) {
                if (match_bitapScore(settings, d, static_cast<size_t>(static_cast<int>(loc) + bin_mid), loc, pattern) <= score_threshold) {
                    bin_min = bin_mid;
                } else {
                    bin_max = bin_mid;
                }
                bin_mid = (bin_max - bin_min) / 2 + bin_min;
            }
            // Use the result from this iteration as the maximum for the next.
            bin_max       = bin_mid;
            size_t start  = static_cast<size_t>(max(1, static_cast<int>(loc) - bin_mid + 1));
            size_t finish = static_cast<size_t>(min(static_cast<int>(loc) + bin_mid, static_cast<int>(tl)) + static_cast<int>(pl));

            rd.clear();
            rd.resize(finish + 2, 0);
            rd[finish + 1] = (size_t(1) << d) - 1;
            for (size_t j = finish; j >= start; j--) {
                int charMatch = 0;
                if (tl <= j - 1) {
                    // Out of range.
                    charMatch = 0;
                } else {
                    charMatch = static_cast<int>(s.valueOr(tdata[j - 1], 0));
                }
                if (d == 0) {
                    // First pass: exact match.
                    rd[j] = static_cast<size_t>(((rd[j + 1] << 1) | size_t(1)) & static_cast<size_t>(charMatch));
                } else {
                    // Subsequent passes: fuzzy match.
                    rd[j] = static_cast<size_t>((((rd[j + 1] << 1) | size_t(1)) & static_cast<size_t>(charMatch)) | (((last_rd[j + 1] | last_rd[j]) << 1) | size_t(1)) | last_rd[j + 1]);
                }
                if ((rd[j] & static_cast<size_t>(matchmask)) != 0) {
                    double score = match_bitapScore(settings, d, j - 1, loc, pattern);
                    // This match will almost certainly be better than any existing
                    // match.  But check anyway.
                    if (score <= score_threshold) {
                        // Told you so.
                        score_threshold = score;
                        best_loc        = j - 1;
                        if (best_loc > loc) {
                            // When passing loc, don't exceed our current distance from loc.
                            start = static_cast<size_t>(max(1, 2 * static_cast<int>(loc) - static_cast<int>(best_loc)));
                        } else {
                            // Already passed loc, downhill from here on in.
                            break;
                        }
                    }
                }
            }
            if (match_bitapScore(settings, d + 1, loc, loc, pattern) > score_threshold) {
                // No hope for a (better) match at greater error levels.
                break;
            }
            utils::swap(last_rd, rd);
        }
        return best_loc;
    }

    /**
     * Compute and return the score for a match with e errors and x location.
     * @param e Number of errors in match.
     * @param x Location of match.
     * @param loc Expected location of match.
     * @param pattern Pattern being sought.
     * @return Overall score for match (0.0 = good, 1.0 = bad).
     */
private:
    inline static constexpr double match_bitapScore(const settings_t& settings, size_t e, size_t x, size_t loc, string_view_t pattern) noexcept {
        using namespace dmp::utils;

        float accuracy  = static_cast<float>(e) / static_cast<float>(pattern.length());
        auto  proximity = loc >= x ? loc - x : x - loc;
        if (settings.Match_Distance == 0) {
            // Dodge divide by zero error.
            return proximity == 0 ? static_cast<double>(accuracy) : 1.0;
        }
        return static_cast<double>(accuracy + (static_cast<float>(proximity) / static_cast<float>(settings.Match_Distance)));
    }

    /**
     * Initialise the alphabet for the Bitap algorithm.
     * @param pattern The text to encode.
     * @return Hash of character locations.
     */
protected:
    inline static constexpr void match_alphabet(string_view_t pattern, alphabet_hash_t& s) noexcept {
        auto l(pattern.length());
        // s.reserve(64);
        size_t i = 0;
        for (auto& c : pattern) {
            s[c] |= size_t(1) << (l - i - 1);
            i++;
        }
    }
};
}  // namespace dmp

#endif
