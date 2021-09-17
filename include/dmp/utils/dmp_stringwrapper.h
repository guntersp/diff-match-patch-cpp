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


#ifndef DIFF_MATCH_PATCH_STRINGWRAPPER_H
#define DIFF_MATCH_PATCH_STRINGWRAPPER_H


#include "dmp/utils/dmp_utils.h"

#include <utility>

namespace dmp {
namespace utils {


template <typename string_wrapper, typename child_string_t>
struct child_string_wrapper_helper {
    using type = child_string_t;
};

template <typename string_wrapper>
struct child_string_wrapper_helper<string_wrapper, void> {
    using type = string_wrapper;
};


template <typename _string_t, typename _char_t, typename child_string_t = void>
struct string_wrapper {
    using self_type = string_wrapper<_string_t, child_string_t>;

    using string_t        = _string_t;
    using string_return_t = typename child_string_wrapper_helper<self_type, child_string_t>::type;

    using char_t = _char_t;

    string_t s;

    inline constexpr string_wrapper() noexcept {}

    inline constexpr string_wrapper(const string_t& _s) noexcept
        : s(_s) {}
    inline constexpr string_wrapper(string_t&& _s) noexcept
        : s(std::move(_s)) {}

    inline constexpr string_wrapper(const string_wrapper& _s) noexcept
        : s(_s.s) {}
    inline constexpr string_wrapper(string_wrapper&& _s) noexcept
        : s(std::move(_s.s)) {}


    inline constexpr string_wrapper(char_t _s) noexcept
        : s(&_s, 1) {}
    inline constexpr string_wrapper(const char_t* _s) noexcept
        : s(_s, stringLength(_s)) {}
    inline constexpr string_wrapper(const char_t* _s, size_t length) noexcept
        : s(_s, length) {}

    template <typename other_string_t, std::enable_if_t<!std::is_same_v<other_string_t, string_t> &&            //
                                                            !std::is_same_v<other_string_t, string_wrapper> &&  //
                                                            !std::is_same_v<other_string_t, char_t>,
                                                        int> = 0>
    inline constexpr string_wrapper(const other_string_t& o) noexcept {
        using namespace dmp::utils;
        str_assign(o, *this);
    }

    inline constexpr operator string_t&() noexcept { return s; }
    inline constexpr operator const string_t&() const noexcept { return s; }

    inline constexpr string_wrapper& operator=(const string_t& _s) noexcept {
        if (&_s != &this->s) {
            this->s = _s;
        }
        return *this;
    }

    inline constexpr string_wrapper& operator=(string_t&& _s) noexcept {
        if (&_s != &this->s) {
            this->_s = std::move(s);
        }
        return *this;
    }

    inline constexpr string_wrapper& operator=(const string_wrapper& _s) noexcept {
        if (&_s != this) {
            this->s = _s.s;
        }
        return *this;
    }

    inline constexpr string_wrapper& operator=(string_wrapper&& _s) noexcept {
        if (&_s != this) {
            this->s = std::move(_s.s);
        }
        return *this;
    }

    template <typename other_string_t, std::enable_if_t<!std::is_same_v<other_string_t, string_t> &&            //
                                                            !std::is_same_v<other_string_t, string_wrapper> &&  //
                                                            !std::is_same_v<other_string_t, char_t>,
                                                        int> = 0>
    inline constexpr string_wrapper& operator=(const other_string_t& o) noexcept {
        using namespace dmp::utils;
        str_assign(o, *this);
        return *this;
    }

    inline constexpr size_t length() const noexcept {
        using namespace dmp::utils;
        return get_length(s);
    }

    inline constexpr auto* data() const noexcept {
        using namespace dmp::utils;
        return get_data(s);
    }

    inline constexpr bool empty() const noexcept { return length() == 0; }

    inline constexpr bool operator==(char_t o) const noexcept { return s == o; }
    inline constexpr bool operator==(const char_t* o) const noexcept { return s == o; }
    inline constexpr bool operator==(const string_wrapper& o) const noexcept { return s == o.s; }
    inline constexpr bool operator==(const string_t& o) const noexcept { return s == o; }


    inline constexpr bool operator!=(char_t o) const noexcept { return !operator==(o); }
    inline constexpr bool operator!=(const char_t* o) const noexcept { return !operator==(o); }
    inline constexpr bool operator!=(const string_wrapper& o) const noexcept { return !operator==(o.s); }
    inline constexpr bool operator!=(const string_t& o) const noexcept { return !operator==(o); }


    inline constexpr bool operator<(const string_wrapper& o) const noexcept { return s < o.s; }
    inline constexpr bool operator<(const string_t& o) const noexcept { return s < o; }


    inline constexpr string_wrapper substring(size_t pos) const noexcept {
        using namespace dmp::utils;
        return get_substring(s, pos);
    }
    inline constexpr string_wrapper substring(size_t pos, size_t len) const noexcept {
        using namespace dmp::utils;
        return get_substring(s, pos, len);
    }

    inline constexpr size_t indexOf(char_t needle) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle);
    }
    inline constexpr size_t indexOf(const char_t* needle) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle);
    }
    inline constexpr size_t indexOf(const string_t& needle) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle);
    }
    inline constexpr size_t indexOf(const string_wrapper& needle) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle.s);
    }

    inline constexpr size_t indexOf(char_t needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle, start);
    }
    inline constexpr size_t indexOf(const char_t* needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle, start);
    }
    inline constexpr size_t indexOf(const string_t& needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle, start);
    }
    inline constexpr size_t indexOf(const string_wrapper& needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_indexOf(s, needle.s, start);
    }

    inline constexpr size_t lastIndexOf(char_t needle) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle);
    }
    inline constexpr size_t lastIndexOf(const char_t* needle) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle);
    }
    inline constexpr size_t lastIndexOf(const string_t& needle) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle);
    }
    inline constexpr size_t lastIndexOf(const string_wrapper& needle) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle.s);
    }

    inline constexpr size_t lastIndexOf(char_t needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle, start);
    }
    inline constexpr size_t lastIndexOf(const char_t* needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle, start);
    }
    inline constexpr size_t lastIndexOf(const string_t& needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle, start);
    }
    inline constexpr size_t lastIndexOf(const string_wrapper& needle, size_t start) const noexcept {
        using namespace dmp::utils;
        return get_lastIndexOf(s, needle.s, start);
    }


    inline constexpr bool startsWith(char_t substr) const noexcept { return length() >= 1 && substring(0, 1) == substr; }
    inline constexpr bool startsWith(const string_t& substr) const noexcept {
        using namespace dmp::utils;
        auto l(substr.length());
        return length() >= l && substring(0, l) == substr;
    }
    inline constexpr bool startsWith(const string_wrapper& substr) const noexcept {
        auto l(substr.length());
        return length() >= l && substring(0, l) == substr;
    }

    inline constexpr bool endsWith(char_t substr) const noexcept {
        auto l1(length());
        return l1 >= 1 && substring(l1 - 1, 1) == substr;
    }
    inline constexpr bool endsWith(const string_t& substr) const noexcept {
        using namespace dmp::utils;
        auto l1(length());
        auto l(substr.length());
        return l1 >= l && substring(l1 - l, l) == substr;
    }
    inline constexpr bool endsWith(const string_wrapper& substr) const noexcept {
        auto l1(length());
        auto l(substr.length());
        return l1 >= l && substring(l1 - l, l) == substr;
    }


    inline constexpr auto begin() const noexcept {
        using namespace dmp::utils;
        return get_begin(s);
    }
    inline constexpr auto end() const noexcept {
        using namespace dmp::utils;
        return get_end(s);
    }
};


template <typename other_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline static constexpr bool operator==(const other_string_t& o, const string_wrapper<string_t, char_t, child_string_t>& s) noexcept {
    return s.s == o;
}

template <typename other_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline static constexpr bool operator!=(const other_string_t& o, const string_wrapper<string_t, char_t, child_string_t>& s) noexcept {
    return !(s.s == o);
}

template <typename other_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline static constexpr bool operator==(const string_wrapper<string_t, char_t, child_string_t>& s, const other_string_t& o) noexcept {
    return s.s == o;
}

template <typename other_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline static constexpr bool operator!=(const string_wrapper<string_t, char_t, child_string_t>& s, const other_string_t& o) noexcept {
    return !(s.s == o);
}

template <typename other_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline constexpr static void str_assign(const other_string_t& toAssign, string_wrapper<string_t, char_t, child_string_t>& str) noexcept {
    str = string_wrapper<string_t, char_t, child_string_t> { toAssign.data(), toAssign.length() };
}


template <typename owning_string_t, typename string_t, typename char_t, typename child_string_t = void>
inline constexpr static void append(owning_string_t& str, const string_wrapper<string_t, char_t, child_string_t>& toAppend) noexcept {
    append(str, toAppend.s);
}

template <typename Stream, typename string_t, typename char_t, typename child_string_t = void>
inline constexpr Stream& writeToStream(Stream& s, const string_wrapper<string_t, char_t, child_string_t>& str) noexcept {
    s << str.s;
    return s;
}

}  // namespace utils
}  // namespace dmp

#endif
