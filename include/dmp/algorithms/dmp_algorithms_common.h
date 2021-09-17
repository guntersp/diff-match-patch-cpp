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


#ifndef DIFF_MATCH_PATCH_ALGORITHMS_COMMON_H
#define DIFF_MATCH_PATCH_ALGORITHMS_COMMON_H


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
struct diff_match_patch_common {
    using settings_t = types::settings;

    using string_traits = typename all_traits::string_traits;

    using char_traits     = typename string_traits::char_traits;
    using string_view_t   = typename string_traits::string_view_t;
    using owning_string_t = typename string_traits::owning_string_t;
    using char_t          = typename string_traits::char_t;

    static constexpr const size_t npos = string_traits::npos;


    using container_traits = typename all_traits::container_traits;

    template <typename key, typename value>
    using encoding_hash_t = typename container_traits::template encoding_hash<key, value>;

    template <typename type>
    using string_pool_list_t = typename container_traits::template string_pool_list<type>;

    using string_pool_t = typename string_traits::template string_pool_t<string_view_t, owning_string_t, string_pool_list_t>;


    using Operation = typename types::Operation;

    using diff_t = types::diff<string_traits>;
    struct diffs_t : public container_traits::template diffs_list<diff_t> {
        using parent    = typename container_traits::template diffs_list<diff_t>;
        using diff_t    = diff_match_patch_common::diff_t;
        using element_t = diff_t;

        using parent::parent;


        inline constexpr diffs_t(const diffs_t& o) noexcept {
            parent::reserve(o.size());
            for (auto& d : o) {
                parent::push_back(d);
            }
        }


        inline constexpr diffs_t& operator=(const diffs_t& o) noexcept {
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


public:
    inline constexpr diff_match_patch_common() noexcept {};


    /**
     * Determine the common prefix of two strings.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the start of each string.
     */
    inline static constexpr auto diff_commonPrefix(string_view_t text1, string_view_t text2) noexcept {
        // Performance analysis: https://neil.fraser.name/news/2007/10/09/

        using namespace dmp::utils;

        auto n = min(text1.length(), text2.length());
        auto d1(text1.data());
        auto d2(text2.data());
        for (decltype(n) i = 0; i < n; i++) {
            if (d1[i] != d2[i]) {
                return i;
            }
        }
        return n;
    }

    /**
     * Determine the common suffix of two strings.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the end of each string.
     */
    inline static constexpr auto diff_commonSuffix(string_view_t text1, string_view_t text2) noexcept {
        // Performance analysis: https://neil.fraser.name/news/2007/10/09/

        using namespace dmp::utils;

        auto text1_length(text1.length());
        auto text2_length(text2.length());
        auto n = min(text1_length, text2_length);
        auto d1(text1.data());
        auto d2(text2.data());
        for (decltype(n) i = 1; i <= n; i++) {
            if (d1[text1_length - i] != d2[text2_length - i]) {
                return i - 1;
            }
        }
        return n;
    }


    /**
     * Determine if the suffix of one string is the prefix of another.
     * @param text1 First string.
     * @param text2 Second string.
     * @return The number of characters common to the end of the first
     *     string and the start of the second string.
     */
    inline static constexpr size_t diff_commonOverlap(string_view_t text1, string_view_t text2) noexcept {
        using namespace dmp::utils;

        // Cache the text lengths to prevent multiple calls.
        auto text1_length = text1.length();
        auto text2_length = text2.length();
        // Eliminate the null case.
        if (text1_length == 0 || text2_length == 0) {
            return 0;
        }
        // Truncate the longer string.
        if (text1_length > text2_length) {
            text1 = text1.substring(text1_length - text2_length);
        } else if (text1_length < text2_length) {
            text2 = text2.substring(0, text1_length);
        }
        auto text_length = min(text1_length, text2_length);
        // Quick check for the worst case.
        if (text1 == text2) {
            return text_length;
        }

        // Start by looking for a single character match
        // and increase length until no match is found.
        // Performance analysis: https://neil.fraser.name/news/2010/11/04/
        size_t best   = 0;
        size_t length = 1;
        while (true) {
            string_view_t pattern = text1.substring(text_length - length);
            size_t        found   = text2.indexOf(pattern);
            if (found == npos) {
                return best;
            }
            length += found;
            if (found == 0 || text1.substring(text_length - length) == text2.substring(0, length)) {
                best = length;
                length++;
            }
        }

        return 0;
    }


    /**
     * Convert a Diff list into a pretty HTML report.
     * @param diffs List of Diff objects.
     * @return HTML representation.
     */
public:
    template <typename Stream>
    inline static constexpr Stream& diff_prettyHtml(Stream& s, const diffs_t& diffs) noexcept {
        using namespace dmp::utils;

        for (auto& d : diffs) {
            switch (d.operation) {
                case Operation::INSERT:
                    char_traits::write(s, "<ins style=\"background:#e6ffe6;\">");
                    writeHtml<char_traits>(s, d.text);
                    char_traits::write(s, "</ins>");
                    break;
                case Operation::DELETE:
                    char_traits::write(s, "<del style=\"background:#ffe6e6;\">");
                    writeHtml<char_traits>(s, d.text);
                    char_traits::write(s, "</del>");
                    break;
                case Operation::EQUAL:
                    char_traits::write(s, "<span>");
                    writeHtml<char_traits>(s, d.text);
                    char_traits::write(s, "</span>");
                    break;
            }
        }
        return s;
    }

    /**
     * Compute and return the source text (all equalities and deletions).
     * @param diffs List of Diff objects.
     * @return Source text.
     */
public:
    template <typename Stream>
    inline static constexpr Stream& diff_text1(Stream& s, const diffs_t& diffs) noexcept {
        using namespace dmp::utils;

        for (auto& d : diffs) {
            if (d.operation != Operation::INSERT) {
                writeToStream(s, d.text);
            }
        }
        return s;
    }

    /**
     * Compute and return the source text (all equalities and deletions).
     * @param diffs List of Diff objects.
     * @return Source text.
     */
public:
    inline static constexpr string_view_t diff_text1(string_pool_t& pool, const diffs_t& diffs, size_t start = 0) noexcept {
        using namespace dmp::utils;

        size_t n = diffs.size();

        size_t len = 0;
        for (size_t i = start; i < n; i++) {
            auto& d(diffs[i]);

            if (d.operation != Operation::INSERT) {
                len += d.text.length();
            }
        }

        auto s(pool.createNext(len));
        if (!s) {
            return string_view_t {};
        }

        auto p(s);
        auto l(len);

        for (size_t i = start; i < n; i++) {
            auto& d(diffs[i]);

            if (d.operation != Operation::INSERT) {
                str_copy(p, l, d.text);
            }
        }

        return string_view_t { s, len - l };
    }

    /**
     * Compute and return the destination text (all equalities and insertions).
     * @param diffs List of Diff objects.
     * @return Destination text.
     */
public:
    template <typename Stream>
    inline static constexpr Stream& diff_text2(Stream& s, const diffs_t& diffs) noexcept {
        using namespace dmp::utils;

        for (auto& d : diffs) {
            if (d.operation != Operation::DELETE) {
                writeToStream(s, d.text);
            }
        }
        return s;
    }

    /**
     * Compute and return the source text (all equalities and deletions).
     * @param diffs List of Diff objects.
     * @return Source text.
     */
public:
    inline static constexpr string_view_t diff_text2(string_pool_t& pool, const diffs_t& diffs, size_t start = 0) noexcept {
        using namespace dmp::utils;

        size_t n = diffs.size();

        size_t len = 0;
        for (size_t i = start; i < n; i++) {
            auto& d(diffs[i]);

            if (d.operation != Operation::DELETE) {
                len += d.text.length();
            }
        }

        auto s(pool.createNext(len));
        if (!s) {
            return string_view_t {};
        }

        auto p(s);
        auto l(len);

        for (size_t i = start; i < n; i++) {
            auto& d(diffs[i]);

            if (d.operation != Operation::DELETE) {
                str_copy(p, l, d.text);
            }
        }

        return string_view_t { s, len - l };
    }


    /**
     * Compute the Levenshtein distance; the number of inserted, deleted or
     * substituted characters.
     * @param diffs List of Diff objects.
     * @return Number of changes.
     */
public:
    inline static constexpr size_t diff_levenshtein(const diffs_t& diffs) noexcept {
        using namespace dmp::utils;

        size_t levenshtein = 0;
        size_t insertions  = 0;
        size_t deletions   = 0;
        for (auto& d : diffs) {
            switch (d.operation) {
                case Operation::INSERT:
                    insertions += d.text.length();
                    break;
                case Operation::DELETE:
                    deletions += d.text.length();
                    break;
                case Operation::EQUAL:
                    // A deletion and an insertion is one substitution.
                    levenshtein += max(insertions, deletions);
                    insertions = 0;
                    deletions  = 0;
                    break;
            }
        }
        levenshtein += max(insertions, deletions);
        return levenshtein;
    }


    /**
     * Crush the diff into an encoded string which describes the operations
     * required to transform text1 into text2.
     * E.g. =3\t-2\t+ing  -> Keep 3 chars, delete 2 chars, insert 'ing'.
     * Operations are tab-separated.  Inserted text is escaped using %xx
     * notation.
     * @param diffs Array of Diff objects.
     * @return Delta text.
     */
public:
    template <typename Stream>
    inline static constexpr Stream& diff_toDelta(Stream& s, const diffs_t& diffs) noexcept {
        bool first = true;

        using namespace dmp::utils;

        for (auto& d : diffs) {
            if (first) {
                first = false;
            } else {
                char_traits::write(s, '\t');
            }
            switch (d.operation) {
                case Operation::INSERT:
                    char_traits::write(s, '+');
                    encodeURI<char_traits>(s, d.text);
                    break;
                case Operation::DELETE:
                    char_traits::write(s, '-');
                    writeToStream(s, d.text.length());
                    break;
                case Operation::EQUAL:
                    char_traits::write(s, '=');
                    writeToStream(s, d.text.length());
                    break;
            }
        }
        return s;
    }

public:
    inline static constexpr bool nextLine(string_view_t& line, const string_view_t& text, size_t& start, size_t length, char_t separator) noexcept {
        if (start >= length) {
            return false;
        }

        size_t p(text.indexOf(separator, start));
        if (p == npos) {
            p = length;
        }
        line  = text.substring(start, p - start);
        start = p + 1;
        return true;
    }
};
}  // namespace dmp

#endif
