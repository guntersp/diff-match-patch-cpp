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


#ifndef DIFF_MATCH_PATCH_TESTS_H
#define DIFF_MATCH_PATCH_TESTS_H


#include "utils/dmp_utils.h"


#ifndef STR
// this converts latin1 char strings to the string type used in the test. Since the string might be used outside of a function argument the temp
// string needs to be stored in a pool
#    define STR(S) (pool.createFromCCharString(S))
#endif

namespace dmp {
namespace tests {


#define DEFINE_GTEST(Traits, Group, Test)                                                                                                            \
    TEST(Group##_##Traits, Test) { dmp::tests::Group<dmp::tests::test_traits<traits>>::Test(); }

#define assertEquals(Comment, Value1, Value2) EXPECT_EQ(Value1, Value2)
#define assertFalse(Comment, Value1) EXPECT_FALSE(Value1)
#define assertTrue(Comment, Value1) EXPECT_TRUE(Value1)
#define assertGreater(Comment, Value1, Value2) EXPECT_GT(Value1, Value2)
#define assertLessEqual(Comment, Value1, Value2) EXPECT_LE(Value1, Value2)


template <typename all_traits>
struct diff_match_patch_test;

template <typename type>
struct vector;


template <typename all_traits>
struct test_traits {
    using dmp_t = dmp::tests::diff_match_patch_test<all_traits>;

    using string_traits        = typename dmp_t::string_traits;
    using string_view_t        = typename dmp_t::string_view_t;
    using owning_string_t      = typename dmp_t::owning_string_t;
    using long_owning_string_t = typename dmp_t::long_owning_string_t;
    using string_pool_t        = typename dmp_t::string_pool_t;

    using char_traits    = typename dmp_t::char_traits;
    using char_t         = typename dmp_t::char_t;
    using stringstream_t = typename dmp_t::stringstream_t;

    using encoding_algorithm_diff_t = typename dmp_t::encoding_algorithm_diff_t;
    using encoding_owning_string_t  = typename dmp_t::encoding_owning_string_t;
    using encoding_char_t           = typename dmp_t::encoding_char_t;
    using encoding_string_view_t    = typename dmp_t::encoding_string_view_t;
    using encoding_list_t           = typename dmp_t::encoding_list_t;

    using HalfMatchResult = typename dmp_t::HalfMatchResult;
    using alphabet_hash_t = typename all_traits::container_traits::template alphabet_hash<char_t, size_t>;

    using Operation = typename dmp_t::Operation;
    using Diff      = typename dmp_t::Diff;
    using diffs_t   = typename dmp_t::diffs_t;
    using Diffs     = typename dmp_t::Diffs;
    using patches_t = typename dmp_t::patches_t;
    using Patch     = typename dmp_t::Patch;
    using Patches   = typename dmp_t::Patches;
    using ediff_t   = typename dmp_t::ediff_t;
    using ediffs_t  = typename dmp_t::ediffs_t;
    using char_t    = typename dmp_t::char_t;
    using clock_t   = typename dmp_t::clock_t;
};


template <typename test_traits>
struct DiffMatchPatch_diff : public test_traits {
    using dmp_t = typename test_traits::dmp_t;

    using string_traits        = typename test_traits::string_traits;
    using string_view_t        = typename test_traits::string_view_t;
    using owning_string_t      = typename test_traits::owning_string_t;
    using long_owning_string_t = typename test_traits::long_owning_string_t;
    using string_pool_t        = typename test_traits::string_pool_t;

    using char_traits    = typename test_traits::char_traits;
    using stringstream_t = typename test_traits::stringstream_t;

    using encoding_algorithm_diff_t = typename test_traits::encoding_algorithm_diff_t;
    using encoding_owning_string_t  = typename test_traits::encoding_owning_string_t;
    using encoding_char_t           = typename test_traits::encoding_char_t;
    using encoding_string_view_t    = typename test_traits::encoding_string_view_t;
    using encoding_list_t           = typename test_traits::encoding_list_t;

    using HalfMatchResult = typename test_traits::HalfMatchResult;

    using Operation = typename test_traits::Operation;
    using Diff      = typename test_traits::Diff;
    using diffs_t   = typename test_traits::diffs_t;
    using Diffs     = typename test_traits::Diffs;
    using patches_t = typename test_traits::patches_t;
    using Patches   = typename test_traits::Patches;
    using ediff_t   = typename test_traits::ediff_t;
    using ediffs_t  = typename test_traits::ediffs_t;
    using char_t    = typename test_traits::char_t;
    using clock_t   = typename test_traits::clock_t;


    inline static void commonPrefixTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Detect any common suffix.
        assertEquals("diff_commonPrefix: Null case.", 0, dmp.diff_commonPrefix(STR("abc"), STR("xyz")));

        assertEquals("diff_commonPrefix: Non-null case.", 4, dmp.diff_commonPrefix(STR("1234abcdef"), STR("1234xyz")));

        assertEquals("diff_commonPrefix: Whole case.", 4, dmp.diff_commonPrefix(STR("1234"), STR("1234xyz")));
    }

    inline static void commonSuffixTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Detect any common suffix.
        assertEquals("diff_commonSuffix: Null case.", 0, dmp.diff_commonSuffix(STR("abc"), STR("xyz")));

        assertEquals("diff_commonSuffix: Non-null case.", 4, dmp.diff_commonSuffix(STR("abcdef1234"), STR("xyz1234")));

        assertEquals("diff_commonSuffix: Whole case.", 4, dmp.diff_commonSuffix(STR("1234"), STR("xyz1234")));
    }


    inline static void commonOverlapTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Detect any suffix/prefix overlap.
        assertEquals("diff_commonOverlap: Null case.", 0, dmp.diff_commonOverlap(STR(""), STR("abcd")));

        assertEquals("diff_commonOverlap: Whole case.", 3, dmp.diff_commonOverlap(STR("abc"), STR("abcd")));

        assertEquals("diff_commonOverlap: No overlap.", 0, dmp.diff_commonOverlap(STR("123456"), STR("abcd")));

        assertEquals("diff_commonOverlap: Overlap.", 3, dmp.diff_commonOverlap(STR("123456xxx"), STR("xxxabcd")));

        // Some overly clever languages (C#) may treat ligatures as equal to their
        // component letters.  E.g. U+FB01 == 'fi'
        assertEquals("diff_commonOverlap: Unicode.", 0, dmp.diff_commonOverlap(STR("fi"), STR("\xEF\xAC\x81i") /*original: "\ufb01i"*/));
    }

    inline static void halfmatchTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Detect a halfmatch.
        dmp.Diff_Timeout = 1;
        assertFalse("diff_halfMatch: No match #1.", dmp.diff_halfMatchOriginalReturn(STR("1234567890"), STR("abcdef")));

        assertFalse("diff_halfMatch: No match #2.", dmp.diff_halfMatchOriginalReturn(STR("12345"), STR("23")));

        assertEquals("diff_halfMatch: Single Match #1.", HalfMatchResult(STR("12"), STR("90"), STR("a"), STR("z"), STR("345678")),
                     dmp.diff_halfMatch(STR("1234567890"), STR("a345678z")));

        assertEquals("diff_halfMatch: Single Match #2.", HalfMatchResult(STR("a"), STR("z"), STR("12"), STR("90"), STR("345678")),
                     dmp.diff_halfMatch(STR("a345678z"), STR("1234567890")));

        assertEquals("diff_halfMatch: Single Match #3.", HalfMatchResult(STR("abc"), STR("z"), STR("1234"), STR("0"), STR("56789")),
                     dmp.diff_halfMatch(STR("abc56789z"), STR("1234567890")));

        assertEquals("diff_halfMatch: Single Match #4.", HalfMatchResult(STR("a"), STR("xyz"), STR("1"), STR("7890"), STR("23456")),
                     dmp.diff_halfMatch(STR("a23456xyz"), STR("1234567890")));

        assertEquals("diff_halfMatch: Multiple Matches #1.", HalfMatchResult(STR("12123"), STR("123121"), STR("a"), STR("z"), STR("1234123451234")),
                     dmp.diff_halfMatch(STR("121231234123451234123121"), STR("a1234123451234z")));

        assertEquals("diff_halfMatch: Multiple Matches #2.", HalfMatchResult(STR(""), STR("-=-=-=-=-="), STR("x"), STR(""), STR("x-=-=-=-=-=-=-=")),
                     dmp.diff_halfMatch(STR("x-=-=-=-=-=-=-=-=-=-=-=-="), STR("xx-=-=-=-=-=-=-=")));

        assertEquals("diff_halfMatch: Multiple Matches #3.", HalfMatchResult(STR("-=-=-=-=-="), STR(""), STR(""), STR("y"), STR("-=-=-=-=-=-=-=y")),
                     dmp.diff_halfMatch(STR("-=-=-=-=-=-=-=-=-=-=-=-=y"), STR("-=-=-=-=-=-=-=yy")));

        // Optimal diff would be -q+x=H-i+e=lloHe+Hu=llo-Hew+y not -qHillo+x=HelloHe-w+Hulloy
        assertEquals("diff_halfMatch: Non-optimal halfmatch.", HalfMatchResult(STR("qHillo"), STR("w"), STR("x"), STR("Hulloy"), STR("HelloHe")),
                     dmp.diff_halfMatch(STR("qHilloHelloHew"), STR("xHelloHeHulloy")));

        dmp.Diff_Timeout = 0;
        assertFalse("diff_halfMatch: Optimal no halfmatch.", dmp.diff_halfMatchOriginalReturn(STR("qHilloHelloHew"), STR("xHelloHeHulloy")));
    }

    inline static void linesToCharsTest() {
        dmp_t         dmp;
        string_pool_t pool;

        using namespace dmp::utils;

        encoding_owning_string_t result[2] {};

        // Convert lines down to characters.
        encoding_list_t tmpVector;
        encoding_list_t tmpResult;
        tmpVector.push_back(STR(""));
        tmpVector.push_back(STR("alpha\n"));
        tmpVector.push_back(STR("beta\n"));
        dmp.diff_linesToChars(result, STR("alpha\nbeta\nalpha\n"), STR("beta\nalpha\nbeta\n"), tmpResult);
        encoding_char_t arr1[] = { L'\u0001', L'\u0002', L'\u0001' };
        assertEquals("diff_linesToChars: Shared lines #1.", encoding_string_view_t(arr1, dmp::utils::array_size(arr1)), result[0]);
        encoding_char_t arr2[] = { L'\u0002', L'\u0001', L'\u0002' };
        assertEquals("diff_linesToChars: Shared lines #2.", encoding_string_view_t(arr2, dmp::utils::array_size(arr2)), result[1]);
        assertEquals("diff_linesToChars: Shared lines #3.", tmpVector, tmpResult);

        tmpVector.clear();
        tmpVector.push_back(STR(""));
        tmpVector.push_back(STR("alpha\r\n"));
        tmpVector.push_back(STR("beta\r\n"));
        tmpVector.push_back(STR("\r\n"));
        dmp.diff_linesToChars(result, STR(""), STR("alpha\r\nbeta\r\n\r\n\r\n"), tmpResult);
        assertEquals("diff_linesToChars: Empty string and blank lines #1.", encoding_string_view_t {}, result[0]);
        encoding_char_t arr3[] = { L'\u0001', L'\u0002', L'\u0003', L'\u0003' };
        assertEquals("diff_linesToChars: Empty string and blank lines #2.", encoding_string_view_t(arr3, dmp::utils::array_size(arr3)), result[1]);
        assertEquals("diff_linesToChars: Empty string and blank lines #3.", tmpVector, tmpResult);

        tmpVector.clear();
        tmpVector.push_back(STR(""));
        tmpVector.push_back(STR("a"));
        tmpVector.push_back(STR("b"));
        dmp.diff_linesToChars(result, STR("a"), STR("b"), tmpResult);
        assertEquals("diff_linesToChars: No linebreaks #1.", encoding_string_view_t(arr3, 1), result[0]);
        assertEquals("diff_linesToChars: No linebreaks #2.", encoding_string_view_t(&arr3[1], 1), result[1]);
        assertEquals("diff_linesToChars: No linebreaks #3.", tmpVector, tmpResult);

        // More than 256 to reveal any 8-bit limitations.
        size_t n = 300;
        tmpVector.clear();


        long_owning_string_t     lines;
        encoding_owning_string_t chars;
        {
            struct Line {
                size_t pos = 0;
                size_t len = 0;
            };
            vector<Line> lineList;
            for (size_t i = 1; i < n + 1; i++) {
                stringstream_t s;
                writeToStream(s, i);
                char_traits::write(s, '\n');

                size_t l(lines.length());
                lines += s.str();
                lineList.push_back(Line { l, lines.length() - l });

                chars.push_back(static_cast<encoding_char_t>(i));
            }
            auto d(lines.data());
            for (auto& l : lineList) {
                tmpVector.emplace_back(string_view_t { d + l.pos, l.len });
            }
        }
        assertEquals("Test initialization fail #1.", n, tmpVector.size());
        assertEquals("Test initialization fail #2.", n, chars.size());
        tmpVector.push_front(STR(""));
        dmp.diff_linesToChars(result, lines, STR(""), tmpResult);
        assertEquals("diff_linesToChars: More than 256 #1.", chars, result[0]);
        assertEquals("diff_linesToChars: More than 256 #2.", encoding_string_view_t {}, result[1]);
        assertEquals("diff_linesToChars: More than 256 #3.", tmpVector, tmpResult);
    }


    inline static void charsToLinesTest() {
        dmp_t         dmp;
        string_pool_t pool;

        using namespace dmp::utils;

        // First check that Diff equality works.
        assertTrue(STR("diff_charsToLines: Equality #1."), Diff(Operation::EQUAL, STR("a")) == Diff(Operation::EQUAL, STR("a")));

        assertEquals("diff_charsToLines: Equality #2.", Diff(Operation::EQUAL, STR("a")), Diff(Operation::EQUAL, STR("a")));

        // Convert chars up to lines.
        encoding_char_t arr1[] = { L'\u0001', L'\u0002', L'\u0001' };
        encoding_char_t arr2[] = { L'\u0002', L'\u0001', L'\u0002' };
        ediffs_t        ediffs;
        ediffs.addAll(ediff_t(Operation::EQUAL, encoding_string_view_t(arr1, dmp::utils::array_size(arr1))),
                      ediff_t(Operation::INSERT, encoding_string_view_t(arr2, dmp::utils::array_size(arr2))));
        encoding_list_t tmpVector;
        tmpVector.push_back(STR(""));
        tmpVector.push_back(STR("alpha\n"));
        tmpVector.push_back(STR("beta\n"));


        diffs_t diffs;
        dmp.diff_charsToLines(diffs, pool, ediffs, tmpVector);
        diffs_t arr3;
        arr3.push_back(Diff(Operation::EQUAL, STR("alpha\nbeta\nalpha\n")));
        arr3.push_back(Diff(Operation::INSERT, STR("beta\nalpha\nbeta\n")));
        assertEquals("diff_charsToLines: Shared lines.", arr3, diffs);


        // More than 256 to reveal any 8-bit limitations.
        size_t n = 300;
        tmpVector.clear();
        long_owning_string_t     lines;
        encoding_owning_string_t chars;
        {
            struct Line {
                size_t pos = 0;
                size_t len = 0;
            };
            vector<Line> lineList;
            for (size_t i = 1; i < n + 1; i++) {
                stringstream_t s;
                writeToStream(s, i);
                char_traits::write(s, '\n');

                size_t l(lines.length());
                lines += s.str();
                lineList.push_back(Line { l, lines.length() - l });

                chars.push_back(static_cast<encoding_char_t>(i));
            }
            auto d(lines.data());
            for (auto& l : lineList) {
                tmpVector.emplace_back(string_view_t { d + l.pos, l.len });
            }
        }
        assertEquals("Test initialization fail #3.", n, tmpVector.size());
        assertEquals("Test initialization fail #4.", n, chars.size());
        tmpVector.push_front(STR(""));
        diffs.clear();
        ediffs.clear();
        ediffs.addAll(ediff_t(Operation::DELETE, chars));
        dmp.diff_charsToLines(diffs, pool, ediffs, tmpVector);
        arr3.clear();
        arr3.push_back(Diff(Operation::DELETE, lines));
        assertEquals("diff_charsToLines: More than 256.", arr3, diffs);

#ifndef DISABLE_VERY_LONG_STRING_TEST
        // More than 65536 to verify any 16-bit limitation.
        {
            lines.clear();
            n = 66000;
            for (size_t i = 0; i < n; i++) {
                stringstream_t s;
                writeToStream(s, i);
                char_traits::write(s, '\n');

                lines += s.str();
            }
        }
        encoding_list_t          tmpResult;
        encoding_owning_string_t result[2] {};
        dmp.diff_linesToChars(result, lines, STR(""), tmpResult);
        diffs.clear();
        ediffs.clear();
        ediffs.addAll(ediff_t(Operation::INSERT, result[0]));
        dmp.diff_charsToLines(diffs, pool, ediffs, tmpResult);
        assertEquals("diff_charsToLines: More than 65536.", lines, diffs[0].text);
#endif
    }


    inline static void cleanupMergeTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Cleanup a messy diff.
        // Null case.
        diffs_t diffs;
        assertEquals("diff_cleanupMerge: Null case.", diffs_t(), diffs);
        assertEquals("diff_cleanupMerge: Null case.", Diffs(), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("b")), Diff(Operation::INSERT, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: No change case.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("b")), Diff(Operation::INSERT, STR("c"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::EQUAL, STR("b")), Diff(Operation::EQUAL, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Merge equalities.", diffs_t().addAll(Diff(Operation::EQUAL, STR("abc"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::DELETE, STR("b")), Diff(Operation::DELETE, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Merge deletions.", diffs_t().addAll(Diff(Operation::DELETE, STR("abc"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR("a")), Diff(Operation::INSERT, STR("b")), Diff(Operation::INSERT, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Merge insertions.", diffs_t().addAll(Diff(Operation::INSERT, STR("abc"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("b")), Diff(Operation::DELETE, STR("c")),
                     Diff(Operation::INSERT, STR("d")), Diff(Operation::EQUAL, STR("e")), Diff(Operation::EQUAL, STR("f")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Merge interweave.",
                     diffs_t().addAll(Diff(Operation::DELETE, STR("ac")), Diff(Operation::INSERT, STR("bd")), Diff(Operation::EQUAL, STR("ef"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("abc")), Diff(Operation::DELETE, STR("dc")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals(STR("diff_cleanupMerge: Prefix and suffix detection."),
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("d")), Diff(Operation::INSERT, STR("b")),
                                      Diff(Operation::EQUAL, STR("c"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("x")), Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("abc")),
                     Diff(Operation::DELETE, STR("dc")), Diff(Operation::EQUAL, STR("y")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals(STR("diff_cleanupMerge: Prefix and suffix detection with equalities."),
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("xa")), Diff(Operation::DELETE, STR("d")), Diff(Operation::INSERT, STR("b")),
                                      Diff(Operation::EQUAL, STR("cy"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::INSERT, STR("ba")), Diff(Operation::EQUAL, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Slide edit left.", diffs_t().addAll(Diff(Operation::INSERT, STR("ab")), Diff(Operation::EQUAL, STR("ac"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("c")), Diff(Operation::INSERT, STR("ab")), Diff(Operation::EQUAL, STR("a")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Slide edit right.", diffs_t().addAll(Diff(Operation::EQUAL, STR("ca")), Diff(Operation::INSERT, STR("ba"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("b")), Diff(Operation::EQUAL, STR("c")),
                     Diff(Operation::DELETE, STR("ac")), Diff(Operation::EQUAL, STR("x")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Slide edit left recursive.",
                     diffs_t().addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::EQUAL, STR("acx"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("x")), Diff(Operation::DELETE, STR("ca")), Diff(Operation::EQUAL, STR("c")),
                     Diff(Operation::DELETE, STR("b")), Diff(Operation::EQUAL, STR("a")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Slide edit right recursive.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("xca")), Diff(Operation::DELETE, STR("cba"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("b")), Diff(Operation::INSERT, STR("ab")), Diff(Operation::EQUAL, STR("c")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Empty merge.", diffs_t().addAll(Diff(Operation::INSERT, STR("a")), Diff(Operation::EQUAL, STR("bc"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("")), Diff(Operation::INSERT, STR("a")), Diff(Operation::EQUAL, STR("b")));
        dmp.diff_cleanupMerge(diffs, pool);
        assertEquals("diff_cleanupMerge: Empty equality.", diffs_t().addAll(Diff(Operation::INSERT, STR("a")), Diff(Operation::EQUAL, STR("b"))),
                     diffs);
    }


    inline static void cleanupSemanticLosslessTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Slide diffs to match logical boundaries.
        diffs_t diffs;
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Null case.", diffs_t(), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("AAA\r\n\r\nBBB")), Diff(Operation::INSERT, STR("\r\nDDD\r\n\r\nBBB")),
                     Diff(Operation::EQUAL, STR("\r\nEEE")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Blank lines.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("AAA\r\n\r\n")), Diff(Operation::INSERT, STR("BBB\r\nDDD\r\n\r\n")),
                                      Diff(Operation::EQUAL, STR("BBB\r\nEEE"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("AAA\r\nBBB")), Diff(Operation::INSERT, STR(" DDD\r\nBBB")), Diff(Operation::EQUAL, STR(" EEE")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Line boundaries.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("AAA\r\n")), Diff(Operation::INSERT, STR("BBB DDD\r\n")),
                                      Diff(Operation::EQUAL, STR("BBB EEE"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("The c")), Diff(Operation::INSERT, STR("ow and the c")), Diff(Operation::EQUAL, STR("at.")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals(
            "diff_cleanupSemanticLossless: Word boundaries.",
            diffs_t().addAll(Diff(Operation::EQUAL, STR("The ")), Diff(Operation::INSERT, STR("cow and the ")), Diff(Operation::EQUAL, STR("cat."))),
            diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("The-c")), Diff(Operation::INSERT, STR("ow-and-the-c")), Diff(Operation::EQUAL, STR("at.")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals(
            "diff_cleanupSemanticLossless: Alphanumeric boundaries.",
            diffs_t().addAll(Diff(Operation::EQUAL, STR("The-")), Diff(Operation::INSERT, STR("cow-and-the-")), Diff(Operation::EQUAL, STR("cat."))),
            diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("a")), Diff(Operation::EQUAL, STR("ax")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Hitting the start.",
                     diffs_t().addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::EQUAL, STR("aax"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("xa")), Diff(Operation::DELETE, STR("a")), Diff(Operation::EQUAL, STR("a")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Hitting the end.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("xaa")), Diff(Operation::DELETE, STR("a"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("The xxx. The ")), Diff(Operation::INSERT, STR("zzz. The ")), Diff(Operation::EQUAL, STR("yyy.")));
        dmp.diff_cleanupSemanticLossless(diffs, pool);
        assertEquals("diff_cleanupSemanticLossless: Sentence boundaries.",
                     diffs_t().addAll(Diff(Operation::EQUAL, STR("The xxx.")), Diff(Operation::INSERT, STR(" The zzz.")),
                                      Diff(Operation::EQUAL, STR(" The yyy."))),
                     diffs);
    }


    inline static void cleanupSemanticTest() {
        dmp_t         dmp;
        string_pool_t pool;


        // Cleanup semantically trivial equalities.
        // Null case.
        Diffs diffs;
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Null case.", diffs_t(), diffs);
        assertEquals("diff_cleanupSemantic: Null case.", diffs, Diffs());
        assertEquals("diff_cleanupSemantic: Null case.", Diffs(), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("cd")), Diff(Operation::EQUAL, STR("12")),
                     Diff(Operation::DELETE, STR("e")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals(STR("diff_cleanupSemantic: No elimination #1."),
                     Diffs().addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("cd")), Diff(Operation::EQUAL, STR("12")),
                                    Diff(Operation::DELETE, STR("e"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::INSERT, STR("ABC")), Diff(Operation::EQUAL, STR("1234")),
                     Diff(Operation::DELETE, STR("wxyz")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: No elimination #2.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::INSERT, STR("ABC")), Diff(Operation::EQUAL, STR("1234")),
                                    Diff(Operation::DELETE, STR("wxyz"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::EQUAL, STR("b")), Diff(Operation::DELETE, STR("c")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Simple elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::INSERT, STR("b"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::EQUAL, STR("cd")), Diff(Operation::DELETE, STR("e")),
                     Diff(Operation::EQUAL, STR("f")), Diff(Operation::INSERT, STR("g")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Backpass elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abcdef")), Diff(Operation::INSERT, STR("cdfg"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR("1")), Diff(Operation::EQUAL, STR("A")), Diff(Operation::DELETE, STR("B")),
                     Diff(Operation::INSERT, STR("2")), Diff(Operation::EQUAL, STR("_")), Diff(Operation::INSERT, STR("1")),
                     Diff(Operation::EQUAL, STR("A")), Diff(Operation::DELETE, STR("B")), Diff(Operation::INSERT, STR("2")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Multiple elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("AB_AB")), Diff(Operation::INSERT, STR("1A2_1A2"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("The c")), Diff(Operation::DELETE, STR("ow and the c")), Diff(Operation::EQUAL, STR("at.")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals(
            "diff_cleanupSemantic: Word boundaries.",
            Diffs().addAll(Diff(Operation::EQUAL, STR("The ")), Diff(Operation::DELETE, STR("cow and the ")), Diff(Operation::EQUAL, STR("cat."))),
            diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("abcxx")), Diff(Operation::INSERT, STR("xxdef")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: No overlap elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abcxx")), Diff(Operation::INSERT, STR("xxdef"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("abcxxx")), Diff(Operation::INSERT, STR("xxxdef")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Overlap elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::EQUAL, STR("xxx")), Diff(Operation::INSERT, STR("def"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("xxxabc")), Diff(Operation::INSERT, STR("defxxx")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Reverse overlap elimination.",
                     Diffs().addAll(Diff(Operation::INSERT, STR("def")), Diff(Operation::EQUAL, STR("xxx")), Diff(Operation::DELETE, STR("abc"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("abcd1212")), Diff(Operation::INSERT, STR("1212efghi")), Diff(Operation::EQUAL, STR("----")),
                     Diff(Operation::DELETE, STR("A3")), Diff(Operation::INSERT, STR("3BC")));
        dmp.diff_cleanupSemantic(diffs);
        assertEquals("diff_cleanupSemantic: Two overlap eliminations.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abcd")), Diff(Operation::EQUAL, STR("1212")), Diff(Operation::INSERT, STR("efghi")),
                                    Diff(Operation::EQUAL, STR("----")), Diff(Operation::DELETE, STR("A")), Diff(Operation::EQUAL, STR("3")),
                                    Diff(Operation::INSERT, STR("BC"))),
                     diffs);
    }


    inline static void cleanupEfficiencyTest() {
        dmp_t         dmp;
        string_pool_t pool;


        // Cleanup operationally trivial equalities.
        dmp.Diff_EditCost = 4;
        Diffs diffs;
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: Null case.", Diffs(), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("wxyz")),
                     Diff(Operation::DELETE, STR("cd")), Diff(Operation::INSERT, STR("34")));
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: No elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("wxyz")),
                                    Diff(Operation::DELETE, STR("cd")), Diff(Operation::INSERT, STR("34"))),
                     diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("xyz")),
                     Diff(Operation::DELETE, STR("cd")), Diff(Operation::INSERT, STR("34")));
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: Four-edit elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abxyzcd")), Diff(Operation::INSERT, STR("12xyz34"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("x")), Diff(Operation::DELETE, STR("cd")),
                     Diff(Operation::INSERT, STR("34")));
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: Three-edit elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("xcd")), Diff(Operation::INSERT, STR("12x34"))), diffs);

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("xy")),
                     Diff(Operation::INSERT, STR("34")), Diff(Operation::EQUAL, STR("z")), Diff(Operation::DELETE, STR("cd")),
                     Diff(Operation::INSERT, STR("56")));
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: Backpass elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abxyzcd")), Diff(Operation::INSERT, STR("12xy34z56"))), diffs);

        dmp.Diff_EditCost = 5;
        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ab")), Diff(Operation::INSERT, STR("12")), Diff(Operation::EQUAL, STR("wxyz")),
                     Diff(Operation::DELETE, STR("cd")), Diff(Operation::INSERT, STR("34")));
        dmp.diff_cleanupEfficiency(diffs);
        assertEquals("diff_cleanupEfficiency: High cost elimination.",
                     Diffs().addAll(Diff(Operation::DELETE, STR("abwxyzcd")), Diff(Operation::INSERT, STR("12wxyz34"))), diffs);
        dmp.Diff_EditCost = 4;
    }


    inline static void prettyHtmlTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Pretty print.
        Diffs diffs;
        diffs.addAll(Diff(Operation::EQUAL, STR("a\n")), Diff(Operation::DELETE, STR("<B>b</B>")), Diff(Operation::INSERT, STR("c&d")));
        assertEquals(STR("diff_prettyHtml:"),
                     STR("<span>a&para;<br></span><del style=\"background:#ffe6e6;\">&lt;B&gt;b&lt;/B&gt;</del><ins "
                         "style=\"background:#e6ffe6;\">c&amp;d</ins>"),
                     dmp.diff_prettyHtml(diffs));
    }


    inline static void textTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Compute the source and destination texts.
        diffs_t diffs;
        diffs.addAll(Diff(Operation::EQUAL, STR("jump")), Diff(Operation::DELETE, STR("s")), Diff(Operation::INSERT, STR("ed")),
                     Diff(Operation::EQUAL, STR(" over ")), Diff(Operation::DELETE, STR("the")), Diff(Operation::INSERT, STR("a")),
                     Diff(Operation::EQUAL, STR(" lazy")));

        assertEquals("Diffext1:", STR("jumps over the lazy"), dmp.diff_text1(diffs));

        assertEquals("Diffext2:", STR("jumped over a lazy"), dmp.diff_text2(diffs));
    }


    inline static void deltaTest() {
        dmp_t         dmp;
        string_pool_t pool;


        // Convert a diff into delta string.
        diffs_t diffs;
        diffs.addAll(Diff(Operation::EQUAL, STR("jump")), Diff(Operation::DELETE, STR("s")), Diff(Operation::INSERT, STR("ed")),
                     Diff(Operation::EQUAL, STR(" over ")), Diff(Operation::DELETE, STR("the")), Diff(Operation::INSERT, STR("a")),
                     Diff(Operation::EQUAL, STR(" lazy")), Diff(Operation::INSERT, STR("old dog")));
        auto text1 = dmp.diff_text1(diffs);
        assertEquals("Diffext1: Base text.", STR("jumps over the lazy"), text1);

        auto delta = dmp.diff_toDelta(diffs);
        assertEquals("DiffoDelta:", STR("=4\t-1\t+ed\t=6\t-3\t+a\t=5\t+old dog"), delta);


        // Convert delta string into a diff.
        diffs_t ds;
        assertTrue(STR("diff_fromDelta: Normal."), dmp.diff_fromDelta(ds, pool, text1, delta));
        assertEquals("diff_fromDelta: Normal.", diffs, ds);


        // Generates error (19 < 20).
        assertFalse("diff_fromDelta: Too long.", dmp.diff_fromDelta(ds, pool, dmp.concat(pool, text1, STR("x")), delta));

        // Generates error (19 > 18).
        assertFalse("diff_fromDelta: Too short.", dmp.diff_fromDelta(ds, pool, string_view_t(text1).substring(1), delta));

        // Generates error (%c3%xy invalid Unicode).
        assertFalse("diff_fromDelta: Invalid character.", dmp.diff_fromDelta(ds, pool, STR(""), STR("+%c3%xy")));

        // Test deltas with special characters.

        char_t zero = 0;
        char_t one  = 1;
        char_t two  = 2;

        if constexpr (sizeof(char_t) <= 1) {
            // note: the original test won't work for 1byte based strings (-> e.g. use wstring instead), diffs would show an incorrect number of chars
            // to remove/add

            // L'\u0680' = '\xda', '\x80'
            char_t s1[] = { static_cast<char_t>('\xda'), static_cast<char_t>('\x80'), ' ', zero, ' ', '\t', ' ', '%' };
            char_t s2[] = { static_cast<char_t>('\xda'), static_cast<char_t>('\x81'), ' ', one, ' ', '\n', ' ', '^' };
            char_t s3[] = { static_cast<char_t>('\xda'), static_cast<char_t>('\x82'), ' ', two, ' ', '\\', ' ', '|' };
            char_t s4[] = { static_cast<char_t>('\xda'), static_cast<char_t>('\x80'), ' ', zero, ' ', '\t', ' ', '%',
                            static_cast<char_t>('\xda'), static_cast<char_t>('\x81'), ' ', one,  ' ', '\n', ' ', '^' };

            diffs.clear();
            diffs.addAll(Diff(Operation::EQUAL, string_view_t(s1, dmp::utils::array_size(s1))),
                         Diff(Operation::DELETE, string_view_t(s2, dmp::utils::array_size(s2))),
                         Diff(Operation::INSERT, string_view_t(s3, dmp::utils::array_size(s3))));
            text1 = dmp.diff_text1(diffs);
            assertEquals("Diffext1: Unicode text.", string_view_t(s4, dmp::utils::array_size(s4)), text1);

            delta = dmp.diff_toDelta(diffs);

            // Lowercase, due to UrlEncode uses lower.
            assertEquals("DiffoDelta: Unicode.", STR("=8\t-8\t+%da%82 %02 %5c %7c"), delta);

            assertTrue(STR("diff_fromDelta: Unicode."), dmp.diff_fromDelta(ds, pool, text1, delta));
            assertEquals("diff_fromDelta: Unicode.", diffs, ds);

        } else {
            char_t s1[] = { static_cast<char_t>(L'\u0680'), ' ', zero, ' ', '\t', ' ', '%' };
            char_t s2[] = { static_cast<char_t>(L'\u0681'), ' ', one, ' ', '\n', ' ', '^' };
            char_t s3[] = { static_cast<char_t>(L'\u0682'), ' ', two, ' ', '\\', ' ', '|' };
            char_t s4[]
                = { static_cast<char_t>(L'\u0680'), ' ', zero, ' ', '\t', ' ', '%', static_cast<char_t>(L'\u0681'), ' ', one, ' ', '\n', ' ', '^' };

            diffs.clear();
            diffs.addAll(Diff(Operation::EQUAL, string_view_t(s1, dmp::utils::array_size(s1))),
                         Diff(Operation::DELETE, string_view_t(s2, dmp::utils::array_size(s2))),
                         Diff(Operation::INSERT, string_view_t(s3, dmp::utils::array_size(s3))));
            text1 = dmp.diff_text1(diffs);
            assertEquals("Diffext1: Unicode text.", string_view_t(s4, dmp::utils::array_size(s4)), text1);

            delta = dmp.diff_toDelta(diffs);

            // Lowercase, due to UrlEncode uses lower.
            assertEquals("DiffoDelta: Unicode.", STR("=7\t-7\t+%da%82 %02 %5c %7c"), delta);

            assertTrue(STR("diff_fromDelta: Unicode."), dmp.diff_fromDelta(ds, pool, text1, delta));
            assertEquals("diff_fromDelta: Unicode.", diffs, ds);
        }


        // Verify pool of unchanged characters.
        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR("A-Z a-z 0-9 - _ . ! ~ * ' ( ) ; / ? : @ & = + $ , # ")));
        auto text2 = dmp.diff_text2(diffs);
        assertEquals("Diffext2: Unchanged characters.", STR("A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , # "), text2);

        delta = dmp.diff_toDelta(diffs);
        assertEquals("DiffoDelta: Unchanged characters.", STR("+A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , # "), delta);

        // Convert delta string into a diff.
        assertTrue(STR("diff_fromDelta: Unchanged characters."), dmp.diff_fromDelta(ds, pool, STR(""), delta));
        assertEquals("diff_fromDelta: Unchanged characters.", diffs, ds);

#ifndef DISABLE_VERY_LONG_STRING_TEST
        // 160 kb string.
        auto a = STR("abcdefghij");
        for (size_t i = 0; i < 14; i++) {
            a = dmp.concat(pool, a, a);
        }
        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, a));
        delta = dmp.diff_toDelta(diffs);
        a     = dmp.concat(pool, STR("+"), a);
        assertEquals("DiffoDelta: 160kb string.", a, delta);

        // Convert delta string into a diff.
        assertTrue(STR("diff_fromDelta: 160kb string."), dmp.diff_fromDelta(ds, pool, STR(""), delta));
        assertEquals("diff_fromDelta: 160kb string.", diffs, ds);
#endif
    }


    inline static void xIndexTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Translate a location in text1 to text2.
        diffs_t diffs;
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("1234")), Diff(Operation::EQUAL, STR("xyz")));
        assertEquals("diff_xIndex: Translation on equality.", 5, dmp.diff_xIndex(diffs, 2));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("1234")), Diff(Operation::EQUAL, STR("xyz")));
        assertEquals("diff_xIndex: Translation on deletion.", 1, dmp.diff_xIndex(diffs, 3));
    }


    inline static void levenshteinTest() {
        dmp_t         dmp;
        string_pool_t pool;

        diffs_t diffs;
        diffs.addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::INSERT, STR("1234")), Diff(Operation::EQUAL, STR("xyz")));
        assertEquals("diff_levenshtein: Levenshtein with trailing equality.", 4, dmp.diff_levenshtein(diffs));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("xyz")), Diff(Operation::DELETE, STR("abc")), Diff(Operation::INSERT, STR("1234")));
        assertEquals("diff_levenshtein: Levenshtein with leading equality.", 4, dmp.diff_levenshtein(diffs));

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("abc")), Diff(Operation::EQUAL, STR("xyz")), Diff(Operation::INSERT, STR("1234")));
        assertEquals("diff_levenshtein: Levenshtein with middle equality.", 7, dmp.diff_levenshtein(diffs));
    }


    inline static void bisectTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Normal.
        auto a = STR("cat");
        auto b = STR("map");
        // Since the resulting diff hasn't been normalized, it would be ok if
        // the insertion and deletion pairs are swapped.
        // If the order changes, tweak this test as required.
        Diffs diffs;
        diffs.addAll(Diff(Operation::DELETE, STR("c")), Diff(Operation::INSERT, STR("m")), Diff(Operation::EQUAL, STR("a")),
                     Diff(Operation::DELETE, STR("t")), Diff(Operation::INSERT, STR("p")));
        assertEquals("diff_bisect: Normal.", diffs, dmp.diff_bisect(a, b, clock_t()));

        // Timeout.
        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("cat")), Diff(Operation::INSERT, STR("map")));
        assertEquals("diff_bisect: Timeout.", diffs, dmp.diff_bisect(a, b, clock_t::now().addMilliseconds(-1)));
    }


    inline static void mainTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Perform a trivial diff.
        Diffs diffs;
        assertEquals("diff_main: Null case.", diffs, dmp.diff_main(STR(""), STR(""), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("abc")));
        assertEquals("diff_main: Equality.", diffs, dmp.diff_main(STR("abc"), STR("abc"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("ab")), Diff(Operation::INSERT, STR("123")), Diff(Operation::EQUAL, STR("c")));
        assertEquals("diff_main: Simple insertion.", diffs, dmp.diff_main(STR("abc"), STR("ab123c"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("123")), Diff(Operation::EQUAL, STR("bc")));
        assertEquals("diff_main: Simple deletion.", diffs, dmp.diff_main(STR("a123bc"), STR("abc"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::INSERT, STR("123")), Diff(Operation::EQUAL, STR("b")),
                     Diff(Operation::INSERT, STR("456")), Diff(Operation::EQUAL, STR("c")));
        assertEquals("diff_main: Two insertions.", diffs, dmp.diff_main(STR("abc"), STR("a123b456c"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("123")), Diff(Operation::EQUAL, STR("b")),
                     Diff(Operation::DELETE, STR("456")), Diff(Operation::EQUAL, STR("c")));
        assertEquals("diff_main: Two deletions.", diffs, dmp.diff_main(STR("a123b456c"), STR("abc"), false));

        // Perform a real diff.
        // Switch off the timeout.
        dmp.Diff_Timeout = 0;
        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("b")));
        assertEquals("diff_main: Simple case #1.", diffs, dmp.diff_main(STR("a"), STR("b"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("Apple")), Diff(Operation::INSERT, STR("Banana")), Diff(Operation::EQUAL, STR("s are a")),
                     Diff(Operation::INSERT, STR("lso")), Diff(Operation::EQUAL, STR(" fruit.")));
        assertEquals("diff_main: Simple case #2.", diffs, dmp.diff_main(STR("Apples are a fruit."), STR("Bananas are also fruit."), false));

        diffs.clear();
        if constexpr (sizeof(char_t) <= 1) {
            // note: the original test won't work for 1byte based strings (-> e.g. use wstring instead)

            char_t c = 0;
            diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR("\xda\x80") /*original: "\u0680"*/),
                         Diff(Operation::EQUAL, STR("x")), Diff(Operation::DELETE, STR("\t")), Diff(Operation::INSERT, string_view_t { &c, 1 }));
            assertEquals("diff_main: Simple case #3.", diffs,
                         dmp.diff_main(STR("ax\t"), dmp.concat(pool, STR("\xda\x80x") /*original: "\u0680"*/, string_view_t { &c, 1 }), false));

        } else {
            char_t c = 0;
            diffs.addAll(Diff(Operation::DELETE, STR("a")), Diff(Operation::INSERT, STR(L"\u0680")), Diff(Operation::EQUAL, STR("x")),
                         Diff(Operation::DELETE, STR("\t")), Diff(Operation::INSERT, string_view_t { &c, 1 }));
            assertEquals("diff_main: Simple case #3.", diffs,
                         dmp.diff_main(STR("ax\t"), dmp.concat(pool, STR(L"\u0680x"), string_view_t { &c, 1 }), false));
        }

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("1")), Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("y")),
                     Diff(Operation::EQUAL, STR("b")), Diff(Operation::DELETE, STR("2")), Diff(Operation::INSERT, STR("xab")));
        assertEquals("diff_main: Overlap #1.", diffs, dmp.diff_main(STR("1ayb2"), STR("abxab"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR("xaxcx")), Diff(Operation::EQUAL, STR("abc")), Diff(Operation::DELETE, STR("y")));
        assertEquals("diff_main: Overlap #2.", diffs, dmp.diff_main(STR("abcy"), STR("xaxcxabc"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("ABCD")), Diff(Operation::EQUAL, STR("a")), Diff(Operation::DELETE, STR("=")),
                     Diff(Operation::INSERT, STR("-")), Diff(Operation::EQUAL, STR("bcd")), Diff(Operation::DELETE, STR("=")),
                     Diff(Operation::INSERT, STR("-")), Diff(Operation::EQUAL, STR("efghijklmnopqrs")),
                     Diff(Operation::DELETE, STR("EFGHIJKLMNOefg")));
        assertEquals("diff_main: Overlap #3.", diffs,
                     dmp.diff_main(STR("ABCDa=bcd=efghijklmnopqrsEFGHIJKLMNOefg"), STR("a-bcd-efghijklmnopqrs"), false));

        diffs.clear();
        diffs.addAll(Diff(Operation::INSERT, STR(" ")), Diff(Operation::EQUAL, STR("a")), Diff(Operation::INSERT, STR("nd")),
                     Diff(Operation::EQUAL, STR(" [[Pennsylvania]]")), Diff(Operation::DELETE, STR(" and [[New")));
        assertEquals("diff_main: Large equality.", diffs, dmp.diff_main(STR("a [[Pennsylvania]] and [[New"), STR(" and [[Pennsylvania]]"), false));


        dmp.Diff_Timeout = 0.005f;  // 1ms
        auto a(STR(
            "`Twas brillig, and the slithy toves\nDid gyre and gimble in the wabe:\nAll mimsy were the borogoves,\nAnd the mome raths outgrabe.\n"));
        auto b(
            STR("I am the very model of a modern major general,\nI've information vegetable, animal, and mineral,\nI know the kings of England, and "
                "I quote the fights historical,\nFrom Marathon to Waterloo, in order categorical.\n"));
#ifndef DISABLE_VERY_LONG_STRING_TEST
        // Increase the text lengths by 1024 times to ensure a timeout.
        for (size_t i = 0; i < 12; i++) {
            a = dmp.concat(pool, a, a);
            b = dmp.concat(pool, b, b);
        }
        clock_t startTime = clock_t::now();
        dmp.diff_main(a, b);
        clock_t endTime = clock_t::now();
        // Test that we took at least the timeout period.
        assertLessEqual(STR("diff_main: Timeout min."), static_cast<long>(dmp.Diff_Timeout * 1000.f), startTime.mSecsTo(endTime));
        // Test that we didn't take forever (be forgiving).
        // Theoretically this test could fail very occasionally if the
        // OS task swaps or locks up for a second at the wrong moment.
        assertGreater(STR("diff_main: Timeout max."), static_cast<long>(dmp.Diff_Timeout * 1000.f) * 100, startTime.mSecsTo(endTime));
#endif
        dmp.Diff_Timeout = 0;

        // Test the linemode speedup.
        // Must be long to pass the 100 char cutoff.
        a = STR(
            "1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234"
            "567890\n1234567890\n");
        b = STR(
            "abcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcd"
            "efghij\nabcdefghij\n");
        assertEquals("diff_main: Simple line-mode.", dmp.diff_main(a, b, true), dmp.diff_main(a, b, false));

        a = STR("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        b = STR("abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij");
        assertEquals("diff_main: Single line-mode.", dmp.diff_main(a, b, true), dmp.diff_main(a, b, false));

        a = STR(
            "1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234"
            "567890\n1234567890\n");
        b = STR(
            "abcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n1234567890\n1234567890\n1234"
            "567890\nabcdefghij\n");
        owning_string_t texts_linemode[2] {};
        dmp.diff_rebuildtexts(dmp.diff_main(a, b, true), texts_linemode[0], texts_linemode[1]);

        owning_string_t texts_textmode[2] {};
        dmp.diff_rebuildtexts(dmp.diff_main(a, b, false), texts_textmode[0], texts_textmode[1]);
        assertEquals("diff_main: Overlap line-mode.", texts_textmode[0], texts_linemode[0]);
        assertEquals("diff_main: Overlap line-mode.", texts_textmode[1], texts_linemode[1]);

        // Test null inputs -- not needed because nulls can't be passed in C#.
    }
};


template <typename test_traits>
struct DiffMatchPatch_match : public test_traits {
    using dmp_t = typename test_traits::dmp_t;

    using string_traits   = typename test_traits::string_traits;
    using string_view_t   = typename test_traits::string_view_t;
    using owning_string_t = typename test_traits::owning_string_t;
    using string_pool_t   = typename test_traits::string_pool_t;

    using char_traits    = typename test_traits::char_traits;
    using stringstream_t = typename test_traits::stringstream_t;

    using alphabet_hash_t = typename test_traits::alphabet_hash_t;

    static constexpr const size_t npos = dmp_t::npos;


    inline static void alphabetTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Initialise the bitmasks for Bitap.
        alphabet_hash_t bitmask;
        bitmask.emplace('a', 4);
        bitmask.emplace('b', 2);
        bitmask.emplace('c', 1);
        alphabet_hash_t m;
        dmp.match_alphabet(STR("abc"), m);
        assertEquals("match_alphabet: Unique.", bitmask, m);

        bitmask.clear();
        bitmask.emplace('a', 37);
        bitmask.emplace('b', 18);
        bitmask.emplace('c', 8);
        m.clear();
        dmp.match_alphabet(STR("abcaba"), m);
        assertEquals("match_alphabet: Duplicates.", bitmask, m);
    }


    inline static void bitapTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Bitap algorithm.
        dmp.Match_Distance  = 100;
        dmp.Match_Threshold = 0.5f;
        assertEquals("match_bitap: Exact match #1.", 5, dmp.match_bitap(STR("abcdefghijk"), STR("fgh"), 5));

        assertEquals("match_bitap: Exact match #2.", 5, dmp.match_bitap(STR("abcdefghijk"), STR("fgh"), 0));

        assertEquals("match_bitap: Fuzzy match #1.", 4, dmp.match_bitap(STR("abcdefghijk"), STR("efxhi"), 0));

        assertEquals("match_bitap: Fuzzy match #2.", 2, dmp.match_bitap(STR("abcdefghijk"), STR("cdefxyhijk"), 5));

        assertEquals("match_bitap: Fuzzy match #3.", npos, dmp.match_bitap(STR("abcdefghijk"), STR("bxy"), 1));

        assertEquals("match_bitap: Overflow.", 2, dmp.match_bitap(STR("123456789xx0"), STR("3456789x0"), 2));

        assertEquals("match_bitap: Before start match.", 0, dmp.match_bitap(STR("abcdef"), STR("xxabc"), 4));

        assertEquals("match_bitap: Beyond end match.", 3, dmp.match_bitap(STR("abcdef"), STR("defyy"), 4));

        assertEquals("match_bitap: Oversized pattern.", 0, dmp.match_bitap(STR("abcdef"), STR("xabcdefy"), 0));

        dmp.Match_Threshold = 0.4f;
        assertEquals("match_bitap: Threshold #1.", 4, dmp.match_bitap(STR("abcdefghijk"), STR("efxyhi"), 1));

        dmp.Match_Threshold = 0.3f;
        assertEquals("match_bitap: Threshold #2.", npos, dmp.match_bitap(STR("abcdefghijk"), STR("efxyhi"), 1));

        dmp.Match_Threshold = 0.0f;
        assertEquals("match_bitap: Threshold #3.", 1, dmp.match_bitap(STR("abcdefghijk"), STR("bcdef"), 1));

        dmp.Match_Threshold = 0.5f;
        assertEquals("match_bitap: Multiple select #1.", 0, dmp.match_bitap(STR("abcdexyzabcde"), STR("abccde"), 3));

        assertEquals("match_bitap: Multiple select #2.", 8, dmp.match_bitap(STR("abcdexyzabcde"), STR("abccde"), 5));

        dmp.Match_Distance = 10;  // Strict location.
        assertEquals("match_bitap: Distance test #1.", npos, dmp.match_bitap(STR("abcdefghijklmnopqrstuvwxyz"), STR("abcdefg"), 24));

        assertEquals("match_bitap: Distance test #2.", 0, dmp.match_bitap(STR("abcdefghijklmnopqrstuvwxyz"), STR("abcdxxefg"), 1));

        dmp.Match_Distance = 1000;  // Loose location.
        assertEquals("match_bitap: Distance test #3.", 0, dmp.match_bitap(STR("abcdefghijklmnopqrstuvwxyz"), STR("abcdefg"), 24));
    }


    inline static void mainTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Full match.
        assertEquals("match_main: Equality.", 0, dmp.match_main(STR("abcdef"), STR("abcdef"), 1000));

        assertEquals("match_main: Null text.", npos, dmp.match_main(STR(""), STR("abcdef"), 1));

        assertEquals("match_main: Null pattern.", 3, dmp.match_main(STR("abcdef"), STR(""), 3));

        assertEquals("match_main: Exact match.", 3, dmp.match_main(STR("abcdef"), STR("de"), 3));

        assertEquals("match_main: Beyond end match.", 3, dmp.match_main(STR("abcdef"), STR("defy"), 4));

        assertEquals("match_main: Oversized pattern.", 0, dmp.match_main(STR("abcdef"), STR("abcdefy"), 0));

        dmp.Match_Threshold = 0.7f;
        assertEquals("match_main: Complex match.", 4, dmp.match_main(STR("I am the very model of a modern major general."), STR(" that berry "), 5));
        dmp.Match_Threshold = 0.5f;

        // Test null inputs -- not needed because nulls can't be passed in C#.
    }
};


template <typename test_traits>
struct DiffMatchPatch_patch : public test_traits {
    using dmp_t = typename test_traits::dmp_t;

    using string_traits   = typename test_traits::string_traits;
    using string_view_t   = typename test_traits::string_view_t;
    using owning_string_t = typename test_traits::owning_string_t;
    using string_pool_t   = typename test_traits::string_pool_t;

    using char_traits    = typename test_traits::char_traits;
    using stringstream_t = typename test_traits::stringstream_t;

    using HalfMatchResult = typename test_traits::HalfMatchResult;

    using Operation = typename test_traits::Operation;
    using Diff      = typename test_traits::Diff;
    using diffs_t   = typename test_traits::diffs_t;
    using Diffs     = typename test_traits::Diffs;
    using patches_t = typename test_traits::patches_t;
    using Patch     = typename test_traits::Patch;
    using Patches   = typename test_traits::Patches;
    using char_t    = typename test_traits::char_t;


    static constexpr const size_t npos = dmp_t::npos;

    inline static void patchObjTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Patch Object.
        Patch p;
        p.start1  = 20;
        p.start2  = 21;
        p.length1 = 18;
        p.length2 = 17;
        p.diffs.addAll(Diff(Operation::EQUAL, STR("jump")), Diff(Operation::DELETE, STR("s")), Diff(Operation::INSERT, STR("ed")),
                       Diff(Operation::EQUAL, STR(" over ")), Diff(Operation::DELETE, STR("the")), Diff(Operation::INSERT, STR("a")),
                       Diff(Operation::EQUAL, STR("\nlaz")));
        auto strp(STR("@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0alaz\n"));

        stringstream_t s;
        p.toString(s);

        assertEquals("Patch: toString.", strp, s.str());
    }


    inline static void fromTextTest() {
        dmp_t         dmp;
        string_pool_t pool;

        assertTrue(STR("patch_fr(omText: #0."), dmp.patch_fromText(STR("")).size() == 0);

        auto strp(STR("@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0alaz\n"));
        auto ps(dmp.patch_fromText(strp));
        assertEquals("patch_fromText: #1.", strp, dmp.toString(ps[0]));

        ps = dmp.patch_fromText(STR("@@ -1 +1 @@\n-a\n+b\n"));
        assertEquals("patch_fromText: #2.", STR("@@ -1 +1 @@\n-a\n+b\n"), dmp.toString(ps[0]));

        ps = dmp.patch_fromText(STR("@@ -1,3 +0,0 @@\n-abc\n"));
        assertEquals("patch_fromText: #3.", STR("@@ -1,3 +0,0 @@\n-abc\n"), dmp.toString(ps[0]));

        ps = dmp.patch_fromText(STR("@@ -0,0 +1,3 @@\n+abc\n"));
        assertEquals("patch_fromText: #4.", STR("@@ -0,0 +1,3 @@\n+abc\n"), dmp.toString(ps[0]));

        // Generates error.
        assertTrue(STR("patch_fromText: #5."), dmp.patch_fromText(STR("Bad\nPatch\n")).isNull());
    }


    inline static void toTextTest() {
        dmp_t         dmp;
        string_pool_t pool;

        auto    strp    = STR("@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n");
        Patches patches = dmp.patch_fromText(strp);
        auto    result  = dmp.patch_toText(patches);
        assertEquals("patch_toText: Single.", strp, result);

        strp    = STR("@@ -1,9 +1,9 @@\n-f\n+F\n oo+fooba\n@@ -7,9 +7,9 @@\n obar\n-,\n+.\n  tes\n");
        patches = dmp.patch_fromText(strp);
        result  = dmp.patch_toText(patches);
        assertEquals("patch_toText: Dual.", strp, result);
    }


    inline static void addContextTest() {
        dmp_t         dmp;
        string_pool_t pool;

        dmp.Patch_Margin = 4;

        auto  ps(dmp.patch_fromText(STR("@@ -21,4 +21,10 @@\n-jump\n+somersault\n")));
        Patch p = ps[0];
        dmp.patch_addContext(p, STR("The quick brown fox jumps over the lazy dog."));
        assertEquals("patch_addContext: Simple case.", STR("@@ -17,12 +17,18 @@\n fox \n-jump\n+somersault\n s ov\n"), dmp.toString(p));

        ps = dmp.patch_fromText(STR("@@ -21,4 +21,10 @@\n-jump\n+somersault\n"));
        p  = ps[0];
        dmp.patch_addContext(p, STR("The quick brown fox jumps."));
        assertEquals("patch_addContext: Not enough trailing context.", STR("@@ -17,10 +17,16 @@\n fox \n-jump\n+somersault\n s.\n"), dmp.toString(p));

        ps = dmp.patch_fromText(STR("@@ -3 +3,2 @@\n-e\n+at\n"));
        p  = ps[0];
        dmp.patch_addContext(p, STR("The quick brown fox jumps."));
        assertEquals("patch_addContext: Not enough leading context.", STR("@@ -1,7 +1,8 @@\n Th\n-e\n+at\n  qui\n"), dmp.toString(p));

        ps = dmp.patch_fromText(STR("@@ -3 +3,2 @@\n-e\n+at\n"));
        p  = ps[0];
        dmp.patch_addContext(p, STR("The quick brown fox jumps.  The quick brown fox crashes."));
        assertEquals("patch_addContext: Ambiguity.", STR("@@ -1,27 +1,28 @@\n Th\n-e\n+at\n  quick brown fox jumps. \n"), dmp.toString(p));
    }


    inline static void makeTest() {
        dmp_t         dmp;
        string_pool_t pool;

        auto patches(dmp.patch_make(STR(""), STR("")));
        assertEquals("patch_make: Null case.", STR(""), dmp.patch_toText(patches));

        auto text1         = STR("The quick brown fox jumps over the lazy dog.");
        auto text2         = STR("That quick brown fox jumped over a lazy dog.");
        auto expectedPatch = STR("@@ -1,8 +1,7 @@\n Th\n-at\n+e\n  qui\n@@ -21,17 +21,18 @@\n jump\n-ed\n+s\n  over \n-a\n+the\n  laz\n");
        // The second patch must be "-21,17 +21,18", not "-22,17 +21,18" due to rolling context.
        patches = dmp.patch_make(text2, text1);
        assertEquals("patch_make: Text2+Text1 inputs.", expectedPatch, dmp.patch_toText(patches));

        expectedPatch = STR("@@ -1,11 +1,12 @@\n Th\n-e\n+at\n  quick b\n@@ -22,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n");
        patches       = dmp.patch_make(text1, text2);
        assertEquals("patch_make: Text1+Text2 inputs.", expectedPatch, dmp.patch_toText(patches));

        auto diffs = dmp.diff_main(text1, text2, false);
        patches    = dmp.patch_make(diffs);
        assertEquals("patch_make: Diff input.", expectedPatch, dmp.patch_toText(patches));

        patches = dmp.patch_make(text1, diffs);
        assertEquals("patch_make: Text1+Diff inputs.", expectedPatch, dmp.patch_toText(patches));

        patches = dmp.patch_make(text1, text2, diffs);
        assertEquals("patch_make: Text1+Text2+Diff inputs (deprecated).", expectedPatch, dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("`1234567890-=[]\\;',./"), STR("~!@#$%^&*()_+{}|:\"<>?"));
        assertEquals("patch_toText: Character encoding.",
                     STR("@@ -1,21 +1,21 @@\n-%601234567890-=%5b%5d%5c;',./\n+~!@#$%25%5e&*()_+%7b%7d%7c:%22%3c%3e?\n"), dmp.patch_toText(patches));

        diffs.clear();
        diffs.addAll(Diff(Operation::DELETE, STR("`1234567890-=[]\\;',./")), Diff(Operation::INSERT, STR("~!@#$%^&*()_+{}|:\"<>?")));
        assertEquals("patch_fromText: Character decoding.", diffs,
                     dmp.patch_fromText(STR("@@ -1,21 +1,21 @@\n-%601234567890-=%5B%5D%5C;',./\n+~!@#$%25%5E&*()_+%7B%7D%7C:%22%3C%3E?\n"))[0].diffs);

#ifndef DISABLE_VERY_LONG_STRING_TEST
        stringstream_t s;
        for (size_t x = 0; x < 100; x++) {
            char_traits::write(s, "abcdef");
        }
        auto text3(s.str());

        char_traits::write(s, "123");
        auto text4(s.str());

        expectedPatch = STR("@@ -573,28 +573,31 @@\n cdefabcdefabcdefabcdefabcdef\n+123\n");
        patches       = dmp.patch_make(text3, text4);
        assertEquals("patch_make: Long string with repeats.", expectedPatch, dmp.patch_toText(patches));
#endif

        // Test null inputs -- not needed because nulls can't be passed in C#.
    }


    inline static void splitMaxTest() {
        dmp_t         dmp;
        string_pool_t pool;

        // Assumes that Match_MaxBits is 32.

        auto patches(dmp.patch_make(STR("abcdefghijklmnopqrstuvwxyz01234567890"), STR("XabXcdXefXghXijXklXmnXopXqrXstXuvXwxXyzX01X23X45X67X89X0")));
        dmp.patch_splitMax(patches);
        assertEquals("patch_splitMax: #1.",
                     STR("@@ -1,32 +1,46 @@\n+X\n ab\n+X\n cd\n+X\n ef\n+X\n gh\n+X\n ij\n+X\n kl\n+X\n mn\n+X\n op\n+X\n qr\n+X\n st\n+X\n uv\n+X\n "
                         "wx\n+X\n yz\n+X\n 012345\n@@ -25,13 +39,18 @@\n zX01\n+X\n 23\n+X\n 45\n+X\n 67\n+X\n 89\n+X\n 0\n"),
                     dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("abcdef1234567890123456789012345678901234567890123456789012345678901234567890uvwxyz"), STR("abcdefuvwxyz"));
        auto oldToText = dmp.patch_toText(patches);
        dmp.patch_splitMax(patches);
        assertEquals("patch_splitMax: #2.", oldToText, dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("1234567890123456789012345678901234567890123456789012345678901234567890"), STR("abc"));
        dmp.patch_splitMax(patches);
        assertEquals(STR("patch_splitMax: #3."),
                     STR("@@ -1,32 +1,4 @@\n-1234567890123456789012345678\n 9012\n@@ -29,32 +1,4 @@\n-9012345678901234567890123456\n 7890\n@@ -57,14 "
                         "+1,3 @@\n-78901234567890\n+abc\n"),
                     dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("abcdefghij , h : 0 , t : 1 abcdefghij , h : 0 , t : 1 abcdefghij , h : 0 , t : 1"),
                                 STR("abcdefghij , h : 1 , t : 1 abcdefghij , h : 1 , t : 1 abcdefghij , h : 0 , t : 1"));
        dmp.patch_splitMax(patches);
        assertEquals(
            STR("patch_splitMax: #4."),
            STR("@@ -2,32 +2,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n@@ -29,32 +29,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n"),
            dmp.patch_toText(patches));
    }


    inline static void addPaddingTest() {
        dmp_t         dmp;
        string_pool_t pool;

        auto patches(dmp.patch_make(STR(""), STR("test")));
        assertEquals("patch_addPadding: Both edges full.", STR("@@ -0,0 +1,4 @@\n+test\n"), dmp.patch_toText(patches));
        dmp.patch_addPadding(patches);
        assertEquals("patch_addPadding: Both edges full.", STR("@@ -1,8 +1,12 @@\n %01%02%03%04\n+test\n %01%02%03%04\n"), dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("XY"), STR("XtestY"));
        assertEquals("patch_addPadding: Both edges partial.", STR("@@ -1,2 +1,6 @@\n X\n+test\n Y\n"), dmp.patch_toText(patches));
        dmp.patch_addPadding(patches);
        assertEquals("patch_addPadding: Both edges partial.", STR("@@ -2,8 +2,12 @@\n %02%03%04X\n+test\n Y%01%02%03\n"), dmp.patch_toText(patches));

        patches = dmp.patch_make(STR("XXXXYYYY"), STR("XXXXtestYYYY"));
        assertEquals("patch_addPadding: Both edges none.", STR("@@ -1,8 +1,12 @@\n XXXX\n+test\n YYYY\n"), dmp.patch_toText(patches));
        dmp.patch_addPadding(patches);
        assertEquals("patch_addPadding: Both edges none.", STR("@@ -5,8 +5,12 @@\n XXXX\n+test\n YYYY\n"), dmp.patch_toText(patches));
    }


    inline static void applyTest() {
        dmp_t         dmp;
        string_pool_t pool;

        dmp.Match_Distance        = 1000;
        dmp.Match_Threshold       = 0.5f;
        dmp.Patch_DeleteThreshold = 0.5f;

        auto patches(dmp.patch_make(STR(""), STR("")));
        auto results(dmp.patch_apply(patches, STR("Hello world.")));
        assertEquals("patch_apply: Null case.", 0, results.results.size());
        assertEquals("patch_apply: Null case.", STR("Hello world."), dmp.toString(results));

        patches = dmp.patch_make(STR("The quick brown fox jumps over the lazy dog."), STR("That quick brown fox jumped over a lazy dog."));
        results = dmp.patch_apply(patches, STR("The quick brown fox jumps over the lazy dog."));
        assertEquals("patch_apply: Exact match.", STR("That quick brown fox jumped over a lazy dog.\tTrue\tTrue"), dmp.toString(results));

        results = dmp.patch_apply(patches, STR("The quick red rabbit jumps over the tired tiger."));
        assertEquals("patch_apply: Partial match.", STR("That quick red rabbit jumped over a tired tiger.\tTrue\tTrue"), dmp.toString(results));

        results = dmp.patch_apply(patches, STR("I am the very model of a modern major general."));
        assertEquals("patch_apply: Failed match.", STR("I am the very model of a modern major general.\tFalse\tFalse"), dmp.toString(results));

        patches = dmp.patch_make(STR("x1234567890123456789012345678901234567890123456789012345678901234567890y"), STR("xabcy"));
        results = dmp.patch_apply(patches, STR("x123456789012345678901234567890-----++++++++++-----123456789012345678901234567890y"));
        assertEquals("patch_apply: Big delete, small change.", STR("xabcy\tTrue\tTrue"), dmp.toString(results));

        patches = dmp.patch_make(STR("x1234567890123456789012345678901234567890123456789012345678901234567890y"), STR("xabcy"));
        results = dmp.patch_apply(patches, STR("x12345678901234567890---------------++++++++++---------------12345678901234567890y"));
        assertEquals("patch_apply: Big delete, big change 1.",
                     STR("xabc12345678901234567890---------------++++++++++---------------12345678901234567890y\tFalse\tTrue"),
                     dmp.toString(results));

        dmp.Patch_DeleteThreshold = 0.6f;
        patches                   = dmp.patch_make(STR("x1234567890123456789012345678901234567890123456789012345678901234567890y"), STR("xabcy"));
        results = dmp.patch_apply(patches, STR("x12345678901234567890---------------++++++++++---------------12345678901234567890y"));
        assertEquals("patch_apply: Big delete, big change 2.", STR("xabcy\tTrue\tTrue"), dmp.toString(results));
        dmp.Patch_DeleteThreshold = 0.5f;

        dmp.Match_Threshold = 0.0f;
        dmp.Match_Distance  = 0;
        patches             = dmp.patch_make(STR("abcdefghijklmnopqrstuvwxyz--------------------1234567890"),
                                 STR("abcXXXXXXXXXXdefghijklmnopqrstuvwxyz--------------------1234567YYYYYYYYYY890"));
        results             = dmp.patch_apply(patches, STR("ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------1234567890"));
        assertEquals("patch_apply: Compensate for failed patch.",
                     STR("ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------1234567YYYYYYYYYY890\tFalse\tTrue"), dmp.toString(results));
        dmp.Match_Threshold = 0.5f;
        dmp.Match_Distance  = 1000;

        patches       = dmp.patch_make(STR(""), STR("test"));
        auto patchStr = dmp.patch_toText(patches);
        dmp.patch_apply(patches, STR(""));
        assertEquals("patch_apply: No side effects.", patchStr, dmp.patch_toText(patches));

        patches  = dmp.patch_make(STR("The quick brown fox jumps over the lazy dog."), STR("Woof"));
        patchStr = dmp.patch_toText(patches);
        dmp.patch_apply(patches, STR("The quick brown fox jumps over the lazy dog."));
        assertEquals("patch_apply: No side effects with major delete.", patchStr, dmp.patch_toText(patches));

        patches = dmp.patch_make(STR(""), STR("test"));
        results = dmp.patch_apply(patches, STR(""));
        assertEquals("patch_apply: Edge exact match.", STR("test\tTrue"), dmp.toString(results));

        patches = dmp.patch_make(STR("XY"), STR("XtestY"));
        results = dmp.patch_apply(patches, STR("XY"));
        assertEquals("patch_apply: Near edge exact match.", STR("XtestY\tTrue"), dmp.toString(results));

        patches = dmp.patch_make(STR("y"), STR("y123"));
        results = dmp.patch_apply(patches, STR("x"));
        assertEquals("patch_apply: Edge partial match.", STR("x123\tTrue"), dmp.toString(results));
    }
};

}  // namespace tests
}  // namespace dmp


#endif
