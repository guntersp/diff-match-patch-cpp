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


#ifndef DIFF_MATCH_PATCH_STRINGPOOL_H
#define DIFF_MATCH_PATCH_STRINGPOOL_H


#include "dmp/utils/dmp_stringpool_base.h"

#include <memory>
#include <cstddef>

namespace dmp {
namespace utils {

template <typename string_view_t, typename owning_string_t, template <typename> class list_t, size_t blockSize = 128>
struct string_pool : public string_pool_base<string_view_t, owning_string_t> {
    using parent         = string_pool_base<string_view_t, owning_string_t>;
    using original_texts = typename parent::original_texts;

    template <size_t size>
    inline constexpr string_pool(const original_texts (&texts)[size]) noexcept
        : parent(&staticCreateNext, texts, size) {}

    inline constexpr string_pool(const original_texts* texts, size_t size) noexcept
        : parent(&staticCreateNext, texts, size) {}

    inline constexpr string_pool() noexcept
        : parent(&staticCreateNext, nullptr, 0) {}

    using strings_type = small_vector<owning_string_t, blockSize>;

    struct block {
        strings_type strings;
    };

    list_t<std::unique_ptr<block>> blocks;

    inline constexpr void clear() noexcept { blocks.clear(); }

    inline constexpr auto* createNext(size_t length) noexcept {
        strings_type* strings = nullptr;

        if (blocks.empty()) {
            auto b(std::make_unique<block>());
            strings = &b->strings;
            blocks.push_back(std::move(b));

        } else {
            strings = &blocks.back()->strings;

            if (strings->size() >= strings->capacity()) {
                auto b(std::make_unique<block>());
                strings = &b->strings;
                blocks.push_back(std::move(b));
            }
        }

        strings->resize(strings->size() + 1);
        auto& s(strings->back());

        str_resize(s, length);
        return get_data(s);
    }

    inline static constexpr auto* staticCreateNext(parent& pool, size_t length) noexcept {
        return static_cast<string_pool&>(pool).createNext(length);
    }
};


}  // namespace utils
}  // namespace dmp

#endif
