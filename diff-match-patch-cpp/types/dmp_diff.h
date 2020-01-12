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


#ifndef DIFF_MATCH_PATCH_DIFF_H
#define DIFF_MATCH_PATCH_DIFF_H


namespace dmp {
namespace types {

/**-
 * The data structure representing a diff is a List of Diff objects:
 * {Diff(Operation::DELETE, "Hello"), Diff(Operation::INSERT, "Goodbye"),
 *  Diff(Operation::EQUAL, " world.")}
 * which means: delete "Hello", add "Goodbye" and keep " world."
 */
enum class Operation { DELETE, INSERT, EQUAL };

/**
 * Class representing one diff Operation::
 */
template <typename string_traits>
struct diff {
    using string_view_t = typename string_traits::string_view_t;
    using char_traits   = typename string_traits::char_traits;


    Operation operation = Operation::INSERT;
    // One of: INSERT, DELETE or EQUAL.

    string_view_t text;
    // The text associated with this diff Operation::


    constexpr diff() noexcept = default;

    /**
     * Constructor.  Initializes the diff with the provided values.
     * @param operation One of INSERT, DELETE or EQUAL.
     * @param text The text being applied.
     */
    inline constexpr diff(Operation operation, string_view_t text) noexcept
        : operation(operation)
        , text(text) {
        // Construct a diff with the specified operation and text.
    }

    /**
     * Display a human-readable version of this Diff.
     * @return text version.
     */

    template <typename Stream>
    inline constexpr Stream& toString(Stream& s) const noexcept {
        char_traits::write(s, "Diff(");

        switch (operation) {
            case Operation::DELETE:
                char_traits::write(s, "DELETE");
                break;
            case Operation::INSERT:
                char_traits::write(s, "INSERT");
                break;
            case Operation::EQUAL:
                char_traits::write(s, "EQUAL");
                break;
        }

        char_traits::write(s, ",\"");
        for (auto c : text) {
            if (c == char_traits::eol) {
                char_traits::write(s, "\u00b6");
            } else {
                char_traits::write(s, c);
            }
        }
        char_traits::write(s, "\")");
        return s;
    }

    /**
     * Is this Diff equivalent to another Diff?
     * @param d Another Diff to compare against.
     * @return true or false.
     */
    inline constexpr bool operator==(const diff& o) const noexcept {
        // Return true if the fields match.
        return o.operation == operation && o.text == text;
    }

    inline constexpr bool operator!=(const diff& o) const noexcept { return !(operator==(o)); }
};

}  // namespace types
}  // namespace dmp

#endif
