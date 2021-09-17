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


#ifndef DIFF_MATCH_PATCH_CLOCKTRAITS_CHRONO_H
#define DIFF_MATCH_PATCH_CLOCKTRAITS_CHRONO_H


#include "dmp/utils/dmp_traits.h"

#include <chrono>

namespace dmp {
namespace traits {

using ChronoTimePoint = std::chrono::time_point<std::chrono::system_clock>;

template <>
struct clock_traits<ChronoTimePoint> {
    struct chrono_clock {

        ChronoTimePoint time;

        inline constexpr chrono_clock() noexcept {};

        inline constexpr chrono_clock(const ChronoTimePoint& time) noexcept
            : time(time) {}

        inline static chrono_clock now() noexcept { return { std::chrono::system_clock::now() }; }

        inline constexpr chrono_clock& addMilliseconds(int64_t milliseconds) noexcept {
            time += std::chrono::milliseconds(milliseconds);
            return *this;
        }

        inline bool hitDeadline() const noexcept {
            if (time == ChronoTimePoint()) {
                return false;
            }
            return now().time > time;
        }

        inline constexpr auto mSecsTo(const chrono_clock& other) const noexcept {
            return std::chrono::duration_cast<std::chrono::milliseconds>(other.time - time).count();
        }
    };

    using type = chrono_clock;
};

}  // namespace traits

using chrono_clock_traits = traits::clock_traits<traits::ChronoTimePoint>;
}  // namespace dmp

#endif
