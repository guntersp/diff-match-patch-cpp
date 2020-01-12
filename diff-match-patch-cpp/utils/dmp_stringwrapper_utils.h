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


#ifndef DIFF_MATCH_PATCH_STRINGWRAPPER_UTILS_H
#define DIFF_MATCH_PATCH_STRINGWRAPPER_UTILS_H


#include <utility>

namespace dmp {
namespace utils {

template <typename string_t>
inline static constexpr auto* get_data(string_t& str) noexcept {
    return str.data();
}

template <typename string_t>
inline static constexpr size_t get_length(const string_t& str) noexcept {
    return str.length();
}

template <typename string_t>
inline static constexpr string_t get_substring(const string_t& str, size_t pos) noexcept {
    return (pos == str.length()) ? string_t() : str.substr(pos);
}

template <typename string_t>
inline static constexpr string_t get_substring(const string_t& str, size_t pos, size_t len) noexcept {
    return (pos == str.length()) ? string_t() : str.substr(pos, len);
}

template <typename string_t, typename Chars>
inline static constexpr size_t get_indexOf(const string_t& str, Chars needle) noexcept {
    return str.find(needle);
}

template <typename string_t, typename Chars>
inline static constexpr size_t get_indexOf(const string_t& str, Chars needle, size_t start) noexcept {
    return str.find(needle, start);
}

template <typename string_t, typename Chars>
inline static constexpr size_t get_lastIndexOf(const string_t& str, Chars needle) noexcept {
    return str.rfind(needle);
}

template <typename string_t, typename Chars>
inline static constexpr size_t get_lastIndexOf(const string_t& str, Chars needle, size_t start) noexcept {
    return str.rfind(needle, start);
}

template <typename string_t, typename owning_string_t>
inline constexpr static void str_assign(const owning_string_t& toAssign, string_t& str) noexcept {
    str = toAssign;
}

template <typename owning_string_t>
inline constexpr static void str_reserve(owning_string_t& str, size_t length) noexcept {
    str.reserve(length);
}

template <typename owning_string_t>
inline constexpr static void str_resize(owning_string_t& str, size_t length) noexcept {
    str.resize(length);
}

template <typename string_t, typename owning_string_t>
inline constexpr static void str_append(owning_string_t& str, const string_t& toAppend) noexcept {
    str += toAppend;
}


}  // namespace utils
}  // namespace dmp

#endif
