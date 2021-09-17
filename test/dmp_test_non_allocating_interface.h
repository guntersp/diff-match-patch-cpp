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


#ifndef DIFF_MATCH_PATCH_TEST_NON_ALLOCATING_INTERFACE_H
#define DIFF_MATCH_PATCH_TEST_NON_ALLOCATING_INTERFACE_H


#include "dmp/diff_match_patch.h"

#include "dmp/traits/dmp_encoding_sub_traits.h"
#include "dmp/utils/dmp_smallvector.h"
#include "dmp/utils/dmp_stringwrapper_utils.h"
#include "dmp/utils/dmp_utils.h"


namespace dmp {
namespace tests {

template <typename type>
struct vector : public dmp::utils::small_vector<type, 384> {
    using parent = dmp::utils::small_vector<type, 384>;

    using parent::parent;
};


template <typename elements_t>
struct container : public dmp::types::container_base<elements_t, elements_t*> {
    using parent = types::container_base<elements_t, elements_t*>;


    using element_t = typename elements_t::element_t;

    elements_t elements;

    inline constexpr container() noexcept
        : parent(&elements) {}

    inline constexpr container(container&& o) noexcept
        : parent(&elements)
        , elements(std::move(o.elements)) {
        this->null = o.null;
    }

    inline constexpr container& operator=(container&& o) noexcept {
        if (this != &o) {
            elements        = std::move(o.elements);
            parent::operator=(&elements);
            this->null      = o.null;
        }
        return *this;
    }

    inline constexpr bool operator==(const container& o) const noexcept { return parent::operator==(o); }
    inline constexpr bool operator==(const elements_t& o) const noexcept { return parent::operator==(o); }
    inline constexpr bool operator!=(const container& o) const noexcept { return !(operator==(o)); }

    inline constexpr bool operator!=(const elements_t& o) const noexcept { return !(operator==(o)); }

    inline constexpr void clear() noexcept { parent::clear(); }

    inline constexpr void reset() noexcept { parent::clear(); }
};

template <typename elements_t>
inline constexpr bool operator==(const elements_t& diffs, const container<elements_t>& dc) noexcept {
    return dc == diffs;
}

template <typename elements_t>
inline constexpr bool operator!=(const elements_t& diffs, const container<elements_t>& dc) noexcept {
    return dc != diffs;
}


template <typename all_traits>
struct diff_match_patch_test : public dmp::types::settings, public dmp::diff_match_patch_base<all_traits> {
    using parent = diff_match_patch_base<all_traits>;

    using commons         = diff_match_patch_common<all_traits>;
    using algorithm_diff  = diff_match_patch_diff<all_traits>;
    using algorithm_match = diff_match_patch_match<all_traits>;
    using algorithm_patch = diff_match_patch_patch<all_traits>;

    using settings_t = typename parent::settings_t;

    using internal_string_pool_t = typename parent::string_pool_t;
    struct string_pool_t {
        inline constexpr string_pool_t() noexcept {}
        inline constexpr string_pool_t(std::nullptr_t) noexcept {}
    };

    using string_view_t        = typename parent::string_view_t;
    using owning_string_t      = typename parent::owning_string_t;
    using long_owning_string_t = typename all_traits::string_traits::long_owning_string_t;
    using stringstream_t       = typename all_traits::string_traits::stringstream_t;
    using clock_t              = typename parent::clock_t;

    using diffs_t = typename parent::diffs_t;
    using Diffs   = container<diffs_t>;

    using patches_t = typename parent::patches_t;
    using Patches   = container<patches_t>;

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

    static constexpr const size_t npos = commons::npos;


    mutable internal_string_pool_t stringPool { nullptr };

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
    inline constexpr auto concat(string_pool_t& /*pool*/, const String1& s1, const String2& s2) const noexcept {
        return stringPool.create(s1, s2);
    }


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
    using parent::diff_main;
    inline Diffs diff_main(string_view_t text1, string_view_t text2) const noexcept {
        // Set a deadline by which time the diff must be complete.
        Diffs container;

        using original_texts = typename internal_string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::diff_main(*this, container.elements, stringPool, text1, text2, true);
        stringPool.resetOriginalTexts();
        return std::move(container);
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
    inline constexpr Diffs diff_main(string_view_t text1, string_view_t text2, bool checklines, bool checkOriginalTexts = true) const noexcept {
        // Set a deadline by which time the diff must be complete.
        Diffs container;
        container.null = false;

        if (checkOriginalTexts) {
            using original_texts = typename internal_string_pool_t::original_texts;
            original_texts texts[] { &text1, &text2 };

            stringPool.setOriginalTexts(texts);

            parent::diff_main(*this, container.elements, stringPool, text1, text2, checklines);
            stringPool.resetOriginalTexts();

        } else {
            parent::diff_main(*this, container.elements, stringPool, text1, text2, checklines);
        }
        return std::move(container);
    }


    /**
     * Reduce the number of edits by eliminating semantically trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    using parent::diff_cleanupSemantic;
    inline constexpr void diff_cleanupSemantic(Diffs& diffs) const noexcept { parent::diff_cleanupSemantic(diffs.elements, stringPool); }

public:
    inline constexpr void diff_cleanupSemanticLossless(diffs_t& diffs, string_pool_t&) const noexcept {
        parent::diff_cleanupSemanticLossless(diffs, stringPool);
    }
    inline constexpr void diff_cleanupMerge(diffs_t& diffs, string_pool_t&) const noexcept { parent::diff_cleanupMerge(diffs, stringPool); }

    /**
     * Reduce the number of edits by eliminating operationally trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    using parent::diff_cleanupEfficiency;
    inline constexpr void diff_cleanupEfficiency(Diffs& diffs) const noexcept { parent::diff_cleanupEfficiency(*this, diffs.elements, stringPool); }


    /**
     * Convert a Diff list into a pretty HTML report.
     * @param diffs List of Diff objects.
     * @return HTML representation.
     */
public:
    inline constexpr owning_string_t diff_prettyHtml(const Diffs& diffs) const noexcept {
        stringstream_t s;
        commons::diff_prettyHtml(s, diffs.elements);
        return s.str();
    }
    inline constexpr owning_string_t diff_prettyHtml(const diffs_t& diffs) const noexcept {
        stringstream_t s;
        commons::diff_prettyHtml(s, diffs);
        return s.str();
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
    inline constexpr owning_string_t diff_toDelta(const Diffs& diffs) const noexcept {
        stringstream_t s;
        commons::diff_toDelta(s, diffs.elements);
        return s.str();
    }
    inline constexpr owning_string_t diff_toDelta(const diffs_t& diffs) const noexcept {
        stringstream_t s;
        commons::diff_toDelta(s, diffs);
        return s.str();
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
    inline constexpr bool diff_fromDelta(diffs_t& diffs, string_pool_t&, string_view_t text1, string_view_t delta) const noexcept {
        using original_texts = typename internal_string_pool_t::original_texts;
        original_texts texts[] { &text1, &delta };

        stringPool.setOriginalTexts(texts);

        bool res = parent::diff_fromDelta(diffs, stringPool, text1, delta);
        stringPool.resetOriginalTexts();
        return res;
    }
    inline constexpr Diffs diff_fromDelta(string_view_t text1, string_view_t delta) const noexcept {
        Diffs container;

        using original_texts = typename internal_string_pool_t::original_texts;
        original_texts texts[] { &text1, &delta };

        stringPool.setOriginalTexts(texts);

        container.null = !parent::diff_fromDelta(container.elements, stringPool, text1, delta);
        stringPool.resetOriginalTexts();

        return std::move(container);
    }


    /**
     * Compute and return the source text (all equalities and deletions).
     * @param diffs List of Diff objects.
     * @return Source text.
     */
public:
    inline constexpr owning_string_t diff_text1(const Diffs& diffs) const noexcept {
        stringstream_t s;
        commons::diff_text1(s, diffs.elements);
        return s.str();
    }
    inline constexpr owning_string_t diff_text1(const diffs_t& diffs) const noexcept {
        stringstream_t s;
        commons::diff_text1(s, diffs);
        return s.str();
    }

    /**
     * Compute and return the destination text (all equalities and insertions).
     * @param diffs List of Diff objects.
     * @return Destination text.
     */
public:
    inline constexpr owning_string_t diff_text2(const Diffs& diffs) const noexcept {
        stringstream_t s;
        commons::diff_text2(s, diffs.elements);
        return s.str();
    }
    inline constexpr owning_string_t diff_text2(const diffs_t& diffs) const noexcept {
        stringstream_t s;
        commons::diff_text2(s, diffs);
        return s.str();
    }


    /**
     * Compute the Levenshtein distance; the number of inserted, deleted or
     * substituted characters.
     * @param diffs List of Diff objects.
     * @return Number of changes.
     */
public:
    inline constexpr size_t diff_levenshtein(const Diffs& diffs) const noexcept { return commons::diff_levenshtein(diffs.elements); }
    inline constexpr size_t diff_levenshtein(const diffs_t& diffs) const noexcept { return commons::diff_levenshtein(diffs); }


    using HalfMatchResult = typename parent::HalfMatchResult;
    using parent::diff_halfMatch;
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

    inline constexpr void diff_charsToLines(diffs_t& diffs, string_pool_t& /*storage*/, const ediffs_t& eDiffs, const encoding_list_t& lines) const
        noexcept {
        parent::diff_charsToLines(diffs, stringPool, eDiffs, lines);
    }

    inline constexpr Diffs diff_bisect(string_view_t text1, string_view_t text2, clock_t deadline) const noexcept {
        Diffs container;

        using original_texts = typename internal_string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::diff_bisect(*this, container.elements, stringPool, text1, text2, deadline);
        stringPool.resetOriginalTexts();
        return std::move(container);
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


    inline constexpr size_t match_main(string_view_t text, string_view_t pattern, size_t loc) const noexcept {
        return parent::match_main(*this, text, pattern, loc);
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
    inline constexpr void patch_splitMax(Patches& patches) const noexcept { parent::patch_splitMax(*this, patches.elements, stringPool); }


    /**
     * Add some padding on text start and end so that edges can match something.
     * Intended to be called only from within patch_apply.
     * @param patches Array of Patch objects.
     * @return The padding string added to each side.
     */
public:
    using parent ::patch_addPadding;
    inline constexpr string_view_t patch_addPadding(Patches& patches) const noexcept {
        return parent::patch_addPadding(*this, patches.elements, stringPool);
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


    /**
     * Take a list of patches and return a textual representation.
     * @param patches List of Patch objects.
     * @return Text representation of patches.
     */
public:
    using parent::patch_toText;

    inline constexpr owning_string_t patch_toText(const Patches& patches) const noexcept {
        stringstream_t s;
        parent::patch_toText(s, patches.elements);
        return s.str();
    }
    inline constexpr owning_string_t patch_toText(const patches_t& patches) const noexcept {
        stringstream_t s;
        parent::patch_toText(s, patches);
        return s.str();
    }


    /**
     * Parse a textual representation of patches and return a List of Patch
     * objects.
     * @param textline Text representation of patches.
     * @return List of Patch objects.
     * @throws ArgumentException If invalid input.
     */
public:
    inline constexpr Patches patch_fromText(string_view_t textline) const noexcept {
        Patches container;

        using original_texts = typename internal_string_pool_t::original_texts;
        original_texts texts[] { &textline };

        stringPool.setOriginalTexts(texts);

        container.null = !parent::patch_fromText(container.elements, stringPool, textline);
        stringPool.resetOriginalTexts();
        return std::move(container);
    }


    /**
     * Compute a list of patches to turn text1 into text2.
     * A set of diffs will be computed.
     * @param text1 Old text.
     * @param text2 New text.
     * @return List of Patch objects.
     */
public:
    using parent::patch_make;
    inline constexpr Patches patch_make(string_view_t text1, string_view_t text2, bool checkOriginalTexts = true) const noexcept {
        Patches container;
        container.null = false;

        if (checkOriginalTexts) {
            using original_texts = typename internal_string_pool_t::original_texts;
            original_texts texts[] { &text1, &text2 };

            stringPool.setOriginalTexts(texts);

            parent::patch_make(static_cast<const settings_t&>(*this), container.elements, stringPool, text1, text2);
            stringPool.resetOriginalTexts();
        } else {
            parent::patch_make(*this, container.elements, stringPool, text1, text2);
        }

        return std::move(container);
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * text1 will be derived from the provided diffs.
     * @param diffs Array of Diff objects for text1 to text2.
     * @return List of Patch objects.
     */
public:
    inline constexpr Patches patch_make(const Diffs& diffs) const noexcept {
        Patches container;
        container.null = diffs.null;
        parent::patch_make(static_cast<const settings_t&>(*this), container.elements, stringPool, diffs.elements);
        return std::move(container);
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
    inline constexpr Patches patch_make(string_view_t text1, string_view_t /*text2*/, const Diffs& diffs) const noexcept {
        return patch_make(text1, diffs);
    }

    /**
     * Compute a list of patches to turn text1 into text2.
     * text2 is not provided, diffs are the delta between text1 and text2.
     * @param text1 Old text.
     * @param diffs Array of Diff objects for text1 to text2.
     * @return List of Patch objects.
     */
public:
    inline constexpr Patches patch_make(string_view_t text1, const Diffs& diffs, bool checkOriginalTexts = true) const noexcept {
        Patches container;
        container.null = diffs.null;

        if (checkOriginalTexts) {
            using original_texts = typename internal_string_pool_t::original_texts;
            original_texts texts[1] { &text1 };

            stringPool.setOriginalTexts(texts);

            parent::patch_make(static_cast<const settings_t&>(*this), container.elements, stringPool, text1, diffs.elements);
            stringPool.resetOriginalTexts();
        } else {
            parent::patch_make(*this, container.elements, stringPool, text1, diffs.elements);
        }

        return std::move(container);
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
    using parent::patch_apply;
    inline constexpr PatchResult patch_apply(const Patches& patches, string_view_t text) const noexcept {
        return parent::patch_apply(*this, patches.elements, stringPool, text);
    }
};

}  // namespace tests
}  // namespace dmp

#endif
