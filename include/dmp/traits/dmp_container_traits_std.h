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


#ifndef DIFF_MATCH_PATCH_CONTAINERTRAITS_STD_H
#define DIFF_MATCH_PATCH_CONTAINERTRAITS_STD_H


#include "dmp/utils/dmp_traits.h"
#include "dmp/utils/dmp_utils.h"

#include <map>
#include <vector>

namespace dmp {
namespace traits {

struct StdContainers;

template <>
struct container_traits<StdContainers> {

    template <typename key, typename value>
    struct encoding_hash : public std::map<key, value> {
        inline value& insertAndReturnValue(const key& k, value v) noexcept { return this->emplace(k, v).first->second; }
    };

    template <typename key, typename value>
    struct alphabet_hash : public std::map<key, value> {
        inline value& insertAndReturnValue(const key& k, value v) noexcept { return this->emplace(k, v).first->second; }

        inline value valueOr(const key& k, value def) const noexcept {
            if (auto it(this->find(k)); it != this->end()) {
                return it->second;
            }

            return def;
        }
    };


    template <typename type>
    using basic_list = std::vector<type>;

    template <typename type>
    struct diffs_list : public basic_list<type> {
        using parent = basic_list<type>;
        using parent::parent;

        template <typename Other>
        inline diffs_list& addAll(const Other& o) noexcept {
            this->insert(this->end(), o.begin(), o.end());
            return *this;
        }

        template <typename... types>
        inline diffs_list& addAll(const type& v, const types&... vs) noexcept {
            this->push_back(v);

            if constexpr (sizeof...(vs) > 0) {
                addAll(vs...);
            }
            return *this;
        }


        inline void push_front(const type& value) noexcept {
            auto s(this->size());
            this->resize(s + 1);

            for (size_t i = s; i > 0; i--) {
                this->operator[](i) = this->operator[](i - 1);
            }
            this->operator[](0) = value;
        }

        inline constexpr void splice(size_t start, size_t count) noexcept {
            using namespace dmp::utils;
            splice_container<type>(*this, start, count);
        }

        template <typename... types>
        inline constexpr void splice(size_t start, size_t count, const type& v, const types&... values) noexcept {
            using namespace dmp::utils;
            splice_container<type>(*this, start, count, v, values...);
        }

        template <typename... types>
        inline constexpr void splice(size_t start, size_t count, type&& value, types&&... values) noexcept {
            using namespace dmp::utils;
            splice_container<type>(*this, start, count, std::forward<type>(value), std::forward<types>(values)...);
        }

        template <size_t size>
        inline constexpr void splice(size_t start, size_t count, const type (&values)[size]) noexcept {
            using namespace dmp::utils;
            splice_container<type, size>(*this, start, count, values, size);
        }

        inline constexpr void splice(size_t start, size_t count, const type* values, size_t size) noexcept {
            using namespace dmp::utils;
            splice_container<type>(*this, start, count, values, size);
        }

        inline constexpr void splice(size_t start, size_t count, const diffs_list& other) noexcept {
            using namespace dmp::utils;
            splice_container<type>(*this, start, count, other.data(), other.size());
        }
    };


    template <typename type>
    using patches_list = diffs_list<type>;

    template <typename type>
    using bisect_list = diffs_list<type>;

    template <typename type>
    using equalities_list = diffs_list<type>;

    template <typename type>
    using string_pool_list = diffs_list<type>;

    template <typename type>
    using encoding_list = diffs_list<type>;

    template <typename type>
    using encoded_string_list = diffs_list<type>;

    template <typename type>
    using match_temp_list = diffs_list<type>;

    template <typename type>
    using patch_result_list = diffs_list<type>;
};

}  // namespace traits

using std_container_traits = traits::container_traits<traits::StdContainers>;

}  // namespace dmp

#endif
