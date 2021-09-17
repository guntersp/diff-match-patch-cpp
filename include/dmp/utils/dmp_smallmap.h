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


#ifndef DIFF_MATCH_PATCH_SMALLMAP_H
#define DIFF_MATCH_PATCH_SMALLMAP_H


#include "dmp/utils/dmp_smallvector.h"


namespace dmp {
namespace utils {


template <typename key, typename value>
struct small_map_item {
    key   first {};
    value second {};

    constexpr small_map_item() noexcept = default;

    inline constexpr small_map_item(key k, value v) noexcept
        : first(k)
        , second(v) {}

    constexpr small_map_item(small_map_item&&) noexcept      = default;
    constexpr small_map_item(const small_map_item&) noexcept = default;

    constexpr small_map_item& operator=(small_map_item&&) noexcept = default;
    constexpr small_map_item& operator=(const small_map_item&) noexcept = default;


    inline constexpr bool operator==(const small_map_item& o) const noexcept { return first == o.first && second == o.second; }
};


template <typename key, typename value, size_t maxItems>
class small_map : private small_vector<small_map_item<key, value>, maxItems> {
public:
    using item_t    = small_map_item<key, value>;
    using parent    = small_vector<item_t, maxItems>;
    using size_type = typename parent::size_type;

public:
    inline constexpr small_map() noexcept {}

    inline constexpr small_map(size_type count) noexcept
        : parent(count) {}

    constexpr explicit small_map(const small_map& other) noexcept = default;
    constexpr small_map(small_map&& other) noexcept               = default;


    inline constexpr item_t*       data() noexcept { return parent::data(); }
    inline constexpr const item_t* data() const noexcept { return parent::data(); }

    inline constexpr size_t size() const noexcept { return parent::size(); }
    inline constexpr bool   empty() const noexcept { return parent::empty(); }

    inline constexpr void clear() noexcept { parent::clear(); }

    inline constexpr void reserve(size_t count) noexcept { parent::reserve(count); }

    inline constexpr size_t capacity() const noexcept { return parent::capacity(); }

    inline constexpr void resize(size_t count) noexcept { parent::resize(count); }

    inline constexpr bool operator==(const small_map& o) const noexcept { return parent ::operator==(o); }

    inline constexpr item_t* find(const key& k) noexcept {
        for (auto& i : *this) {
            if (i.first == k) {
                return &i;
            }
        }

        return end();
    }

    inline constexpr const item_t* find(const key& k) const noexcept {
        for (auto& i : *this) {
            if (i.first == k) {
                return &i;
            }
        }

        return end();
    }

    inline constexpr value& operator[](const key& k) noexcept {
        for (auto& i : *this) {
            if (i.first == k) {
                return i.second;
            }
        }

        return parent::push_back(item_t { k, {} }).second;
    }
    inline constexpr value operator[](const key& k) const noexcept {
        for (auto& i : *this) {
            if (i.first == k) {
                return i.second;
            }
        }

        return {};
    }

    inline constexpr small_map_item<item_t*, bool> emplace(const key& k, const value& v) noexcept {
        for (auto& i : *this) {
            if (i.first == k) {
                return small_map_item<item_t*, bool> { &i, false };
            }
        }

        return small_map_item<item_t*, bool> { &parent::emplace_back(item_t(k, v)), true };
    }


    inline constexpr auto begin() noexcept { return parent::begin(); }
    inline constexpr auto begin() const noexcept { return parent::begin(); }

    inline constexpr auto end() noexcept { return parent::end(); }
    inline constexpr auto end() const noexcept { return parent::end(); }
};

}  // namespace utils
}  // namespace dmp

#endif
