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


#ifndef DIFF_MATCH_PATCH_CONTAINER_H
#define DIFF_MATCH_PATCH_CONTAINER_H


#include "dmp/types/dmp_diff.h"

#include <utility>

namespace dmp {
namespace types {

template <typename elements_t, typename container>
struct container_base {
    container elements;

    using element_t = typename elements_t::element_t;

    bool null = false;

    inline constexpr container_base(container&& elements) noexcept
        : elements(std::move(elements)) {}

    constexpr container_base(container_base&&) noexcept = default;
    constexpr container_base& operator=(container_base&&) noexcept = default;

    inline constexpr bool operator==(const container_base& o) const noexcept {
        if (!elements || !o.elements || null || o.null) {
            return false;
        }
        if (elements == o.elements) {
            return true;
        }
        return *elements == *o.elements;
    }
    inline constexpr bool operator==(const elements_t& o) const noexcept {
        if (!elements || null) {
            return false;
        }
        return *elements == o;
    }
    inline constexpr bool operator!=(const container_base& o) const noexcept { return !(operator==(o)); }

    inline constexpr bool operator!=(const elements_t& o) const noexcept { return !(operator==(o)); }


    inline constexpr auto begin() noexcept { return elements->begin(); }

    inline constexpr auto begin() const noexcept { return elements->begin(); }

    inline constexpr auto end() noexcept { return elements->end(); }

    inline constexpr auto end() const noexcept { return elements->end(); }

protected:
    inline constexpr void clear() noexcept { elements->clear(); }

    inline constexpr void reset() noexcept {
        clear();
        null = true;
    }

public:
    inline constexpr size_t size() const noexcept {
        if (null) {
            return 0;
        }
        return elements->size();
    }
    inline constexpr bool empty() const noexcept { return elements->size() == 0; }

    inline constexpr bool isNull() const noexcept { return null; }


    inline constexpr void reserve(size_t count) noexcept { elements->reserve(count); }

    inline constexpr void push_back(const element_t& value) noexcept {
        if (null) {
            reset();
        }
        elements->push_back(value);
    }

    inline constexpr container_base& addAll(const container_base& o) noexcept {
        for (auto& d : o) {
            push_back(d);
        }
        return *this;
    }

    template <typename... ds>
    inline constexpr container_base& addAll(const element_t& v, const ds&... vs) noexcept {
        push_back(v);

        if constexpr (sizeof...(vs) > 0) {
            addAll(vs...);
        }
        return *this;
    }

    inline constexpr element_t&       operator[](size_t index) noexcept { return (*elements)[index]; }
    inline constexpr const element_t& operator[](size_t index) const noexcept { return (*elements)[index]; }

    inline constexpr element_t&       operator[](int index) noexcept { return (*elements)[static_cast<size_t>(index)]; }
    inline constexpr const element_t& operator[](int index) const noexcept { return (*elements)[static_cast<size_t>(index)]; }
};


}  // namespace types
}  // namespace dmp

#endif
