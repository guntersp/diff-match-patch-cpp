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


#ifndef DIFF_MATCH_PATCH_H
#define DIFF_MATCH_PATCH_H


#include "dmp/diff_match_patch_base.h"
#include "dmp/types/dmp_container.h"

#include <memory>

namespace dmp {

namespace utils {

template <typename elements_t, typename string_pool_t>
struct container : public types::container_base<elements_t, std::unique_ptr<elements_t>> {
    using parent = types::container_base<elements_t, std::unique_ptr<elements_t>>;


    using element_t     = typename elements_t::element_t;
    using string_view_t = typename string_pool_t::string_view_t;

    mutable string_pool_t stringPool;


    inline container() noexcept
        : parent(std::make_unique<elements_t>())
        , stringPool() {}


    container(container&&) noexcept = default;
    container& operator=(container&&) noexcept = default;

    inline constexpr bool operator==(const container& o) const noexcept { return parent::operator==(o); }
    inline constexpr bool operator==(const elements_t& o) const noexcept { return parent::operator==(o); }
    inline constexpr bool operator!=(const container& o) const noexcept { return !(operator==(o)); }

    inline constexpr bool operator!=(const elements_t& o) const noexcept { return !(operator==(o)); }

    inline constexpr void clear() noexcept {
        parent::clear();
        stringPool.clear();
    }

    inline constexpr void reset() noexcept {
        parent::clear();
        stringPool.clear();
    }
};

template <typename elements_t, typename string_pool_t>
inline constexpr bool operator==(const elements_t& diffs, const container<elements_t, string_pool_t>& dc) noexcept {
    return dc == diffs;
}

template <typename elements_t, typename string_pool_t>
inline constexpr bool operator!=(const elements_t& diffs, const container<elements_t, string_pool_t>& dc) noexcept {
    return dc != diffs;
}
}  // namespace utils

/**
 * Class containing the diff, match and patch methods.
 * Also contains the behaviour settings.
 */

template <class all_traits>
struct diff_match_patch : public dmp::types::settings, public dmp::diff_match_patch_base<all_traits> {
    using parent  = dmp::diff_match_patch_base<all_traits>;
    using commons = diff_match_patch_common<all_traits>;

    using settings_t = typename parent::settings_t;

    using string_pool_t = typename parent::string_pool_t;

    using string_view_t   = typename parent::string_view_t;
    using owning_string_t = typename parent::owning_string_t;
    using stringstream_t  = typename all_traits::string_traits::stringstream_t;
    using clock_t         = typename parent::clock_t;

    using diffs_t = typename parent::diffs_t;
    using Diffs   = utils::container<diffs_t, string_pool_t>;

    using patches_t = typename parent::patches_t;
    using Patches   = utils::container<patches_t, string_pool_t>;

    using Operation      = typename parent::Operation;
    using Diff           = typename parent::diff_t;
    using Patch          = typename parent::patch_t;
    using patch_result_t = typename parent::patch_result_t;

    struct PatchResult : public patch_result_t {
        using parent = patch_result_t;

        owning_string_t str;

        inline explicit constexpr PatchResult(const patch_result_t& r) noexcept
            : str(r.text2) {

            using namespace dmp::utils;

            str_assign(str, this->text2);
            this->results = std::move(r.results);
        }

        inline constexpr PatchResult(PatchResult&& o) noexcept
            : str(std::move(o.str)) {

            using namespace dmp::utils;

            str_assign(str, this->text2);
            this->results = std::move(o.results);
        }
        inline constexpr PatchResult& operator=(PatchResult&& o) noexcept {
            if (this != &o) {
                str = std::move(o.str);

                using namespace dmp::utils;

                str_assign(str, this->text2);
                this->results = std::move(o.results);
            }
            return *this;
        }
    };

    static constexpr const size_t npos = commons::npos;


    inline constexpr diff_match_patch() noexcept {}


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
    using parent::diff_main;
    inline Diffs diff_main(string_view_t text1, string_view_t text2) const noexcept {
        // Set a deadline by which time the diff must be complete.
        Diffs container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        container.stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::diff_main(*this, *container.elements, container.stringPool, text1, text2, true);
        container.stringPool.resetOriginalTexts();
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
    inline constexpr Diffs diff_main(string_view_t text1, string_view_t text2, bool checklines) const noexcept {
        // Set a deadline by which time the diff must be complete.
        Diffs container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        container.stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::diff_main(*this, *container.elements, container.stringPool, text1, text2, checklines);
        container.stringPool.resetOriginalTexts();
        return std::move(container);
    }


    /**
     * Reduce the number of edits by eliminating semantically trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    using parent::diff_cleanupSemantic;
    inline constexpr void diff_cleanupSemantic(Diffs& diffs) const noexcept { parent::diff_cleanupSemantic(*diffs.elements, diffs.stringPool); }


    /**
     * Reduce the number of edits by eliminating operationally trivial
     * equalities.
     * @param diffs List of Diff objects.
     */
public:
    using parent::diff_cleanupEfficiency;
    inline constexpr void diff_cleanupEfficiency(Diffs& diffs) const noexcept {
        parent::diff_cleanupEfficiency(*this, *diffs.elements, diffs.stringPool);
    }


    /**
     * Convert a Diff list into a pretty HTML report.
     * @param diffs List of Diff objects.
     * @return HTML representation.
     */
public:
    inline constexpr owning_string_t diff_prettyHtml(const Diffs& diffs) const noexcept {
        stringstream_t s;
        commons::diff_prettyHtml(s, *diffs.elements);
        return s.str();
    }
    inline constexpr owning_string_t diff_prettyHtml(const diffs_t& diffs) const noexcept {
        stringstream_t s;
        commons::diff_prettyHtml(s, diffs);
        return s.str();
    }

    template <typename Stream>
    inline constexpr Stream& diff_prettyHtml(Stream& s, const Diffs& diffs) const noexcept {
        commons::diff_prettyHtml(s, *diffs.elements);
        return s;
    }

    template <typename Stream>
    inline constexpr Stream& diff_prettyHtml(Stream& s, const diffs_t& diffs) const noexcept {
        commons::diff_prettyHtml(s, diffs);
        return s;
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
        commons::diff_toDelta(s, *diffs.elements);
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
    using parent::diff_fromDelta;
    inline constexpr Diffs diff_fromDelta(string_view_t text1, string_view_t delta) const noexcept {
        Diffs container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1, &delta };

        container.stringPool.setOriginalTexts(texts);

        container.null = !parent::diff_fromDelta(*container.elements, container.stringPool, text1, delta);
        container.stringPool.resetOriginalTexts();

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
        commons::diff_text1(s, *diffs.elements);
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
        commons::diff_text2(s, *diffs.elements);
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
    inline constexpr size_t diff_levenshtein(const Diffs& diffs) const noexcept { return commons::diff_levenshtein(*diffs.elements); }
    inline constexpr size_t diff_levenshtein(const diffs_t& diffs) const noexcept { return commons::diff_levenshtein(diffs); }


    /**
     * Locate the best instance of 'pattern' in 'text' near 'loc'.
     * Returns -1 if no match found.
     * @param text The text to search.
     * @param pattern The pattern to search for.
     * @param loc The location to search around.
     * @return Best match index or -1.
     */
public:
    using parent::match_main;
    inline constexpr size_t match_main(string_view_t text, string_view_t pattern, size_t loc) const noexcept {
        return match_main(*this, text, pattern, loc);
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
        parent::patch_toText(s, *patches.elements);
        return s.str();
    }
    inline constexpr owning_string_t patch_toText(const patches_t& patches) const noexcept {
        stringstream_t s;
        parent::patch_toText(s, patches);
        return s.str();
    }

    template <typename Stream>
    inline constexpr Stream& patch_toText(Stream& s, const Patches& patches) const noexcept {
        parent::patch_toText(s, *patches.elements);
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
    inline constexpr Patches patch_fromText(string_view_t textline) const noexcept {
        Patches container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &textline };

        container.stringPool.setOriginalTexts(texts);

        container.null = !parent::patch_fromText(*container.elements, container.stringPool, textline);
        container.stringPool.resetOriginalTexts();
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
    inline constexpr Patches patch_make(string_view_t text1, string_view_t text2) const noexcept {
        Patches container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1, &text2 };

        container.stringPool.setOriginalTexts(texts);

        container.null = false;
        parent::patch_make(static_cast<const settings_t&>(*this), *container.elements, container.stringPool, text1, text2);
        container.stringPool.resetOriginalTexts();
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
        parent::patch_make(static_cast<const settings_t&>(*this), *container.elements, container.stringPool, *diffs.elements);
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
    inline constexpr Patches patch_make(string_view_t text1, const Diffs& diffs) const noexcept {
        Patches container;

        using original_texts = typename string_pool_t::original_texts;
        original_texts texts[] { &text1 };

        container.stringPool.setOriginalTexts(texts);

        container.null = diffs.null;
        parent::patch_make(static_cast<const settings_t&>(*this), *container.elements, container.stringPool, text1, *diffs.elements);
        container.stringPool.resetOriginalTexts();
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
        return PatchResult { parent::patch_apply(*this, *patches.elements, patches.stringPool, text) };
    }
};

}  // namespace dmp


#endif
