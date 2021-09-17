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


#ifndef DIFF_MATCH_PATCH_SMALLVECTOR_H
#define DIFF_MATCH_PATCH_SMALLVECTOR_H


#include "dmp/utils/dmp_utils.h"

#include <utility>

namespace dmp {
namespace utils {

#define DMP_ASSERT(X)                                                                                                                                \
    if (!(X))                                                                                                                                        \
    assert(X)

template <typename type, size_t maxItems>
class small_vector {
public:
    using size_type                        = size_t;
    static constexpr const size_t MaxItems = maxItems;

private:
    type      _data[maxItems];
    size_type _size;

public:
    inline constexpr small_vector() noexcept
        : _data {}
        , _size(0) {}

    inline constexpr small_vector(size_type count) noexcept
        : _data {}
        , _size(count) {
        DMP_ASSERT(count <= maxItems);
    }

    inline constexpr small_vector(size_type count, const type& initializeValue) noexcept
        : _data {}
        , _size(count) {
        DMP_ASSERT(count <= maxItems);
        for (size_type i = 0; i < _size; i++) {
            _data[i] = initializeValue;
        }
    }

    constexpr explicit small_vector(const small_vector& other) noexcept = delete;
    constexpr small_vector(small_vector&& other) noexcept               = default;

    constexpr small_vector& operator=(const small_vector& o) noexcept = delete;
    constexpr small_vector& operator=(small_vector&& o) noexcept = default;


    inline constexpr type*       data() noexcept { return _size == 0 ? nullptr : _data; }
    inline constexpr const type* data() const noexcept { return _size == 0 ? nullptr : _data; }

    inline constexpr size_t size() const noexcept { return _size; }
    inline constexpr bool   empty() const noexcept { return _size == 0; }

    inline constexpr void clear() noexcept { _size = 0; }

    inline constexpr void reserve(size_t count) noexcept {
        DMP_ASSERT(count <= maxItems);
        static_cast<void>(count);
    }

    inline constexpr size_t capacity() const noexcept { return maxItems; }

    inline constexpr void resize(size_t count) noexcept {
        DMP_ASSERT(count <= maxItems);
        _size = count;
    }

    inline constexpr void resize(size_t count, type initialValue) noexcept {
        size_t oldSize(_size);
        resize(count);
        for (size_t i = oldSize; i < count; i++) {
            operator[](i) = initialValue;
        }
    }

    inline constexpr type& operator[](size_t index) noexcept {
        DMP_ASSERT(index >= 0 && index < _size);
        return _data[index];
    }
    inline constexpr const type& operator[](size_t index) const noexcept {
        DMP_ASSERT(index >= 0 && index < _size);
        return _data[index];
    }

    inline constexpr type& operator[](int index) noexcept {
        DMP_ASSERT(index >= 0 && static_cast<size_t>(index) < _size);
        return _data[static_cast<size_t>(index)];
    }
    inline constexpr const type& operator[](int index) const noexcept {
        DMP_ASSERT(index >= 0 && static_cast<size_t>(index) < _size);
        return _data[static_cast<size_t>(index)];
    }


    inline constexpr bool operator==(const small_vector& o) const noexcept {
        if (_size != o._size) {
            return false;
        }

        for (size_t i = 0; i < _size; i++) {
            if (!(operator[](i) == o.operator[](i))) {
                return false;
            }
        }

        return true;
    }

    inline constexpr bool operator!=(const small_vector& o) const noexcept { return !operator==(o); }


    inline constexpr type& push_back(const type& value) noexcept {
        DMP_ASSERT(_size < maxItems);
        auto& v(_data[_size]);
        v = value;
        _size++;
        return v;
    }

    inline constexpr type& push_back(type&& value) noexcept {
        DMP_ASSERT(_size < maxItems);
        auto& v(_data[_size]);
        v = std::move(value);
        _size++;
        return v;
    }

    inline constexpr type& emplace_back(const type& value) noexcept { return push_back(value); }
    inline constexpr type& emplace_back(type&& value) noexcept { return push_back(std::move(value)); }

    template <typename... Arg>
    inline constexpr type& emplace_back(Arg&&... args) noexcept {
        return push_back(type(std::forward<Arg>(args)...));
    }

    inline constexpr type& push_front(const type& value) noexcept {
        DMP_ASSERT(_size < maxItems);
        for (size_t i = _size; i > 0; i--) {
            _data[i] = _data[i - 1];
        }
        auto& v(_data[0]);
        v = value;
        _size++;
        return v;
    }

    inline constexpr type& push_front(type&& value) noexcept {
        DMP_ASSERT(_size < maxItems);
        for (size_t i = _size; i > 0; i--) {
            _data[i] = _data[i - 1];
        }
        auto& v(_data[0]);
        v = std::move(value);
        _size++;
        return v;
    }

    inline constexpr type& emplace_front(const type& value) noexcept { return push_front(value); }

    template <typename... Arg>
    inline constexpr type& emplace_front(Arg&&... args) noexcept {
        return push_front(type(std::forward<Arg>(args)...));
    }

    inline constexpr void pop_back() noexcept {
        DMP_ASSERT(_size > 0);
        _size--;
    }

    inline constexpr type& front() noexcept {
        DMP_ASSERT(_size > 0);
        return _data[0];
    }

    inline constexpr const type& front() const noexcept {
        DMP_ASSERT(_size > 0);
        return _data[0];
    }

    inline constexpr type& back() noexcept {
        DMP_ASSERT(_size > 0);
        return _data[_size - 1];
    }

    inline constexpr const type& back() const noexcept {
        DMP_ASSERT(_size > 0);
        return _data[_size - 1];
    }

    inline constexpr small_vector& addAll(const small_vector& o) noexcept {
        for (auto& d : o) {
            push_back(d);
        }
        return *this;
    }

    template <typename... types>
    inline constexpr small_vector& addAll(const type& v, const types&... vs) noexcept {
        push_back(v);

        if constexpr (sizeof...(vs) > 0) {
            addAll(vs...);
        }
        return *this;
    }


    inline constexpr void splice(size_t start, size_t count) noexcept {
        using namespace dmp::utils;
        splice_container<type>(*this, start, count);
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

    inline constexpr void splice(size_t start, size_t count, const small_vector& other) noexcept {
        using namespace dmp::utils;
        splice_container<type>(*this, start, count, other.data(), other.size());
    }


    inline constexpr auto begin() noexcept { return &_data[0]; }
    inline constexpr auto begin() const noexcept { return &_data[0]; }

    inline constexpr auto end() noexcept { return &_data[0] + static_cast<int>(size()); }
    inline constexpr auto end() const noexcept { return &_data[0] + static_cast<int>(size()); }
};

}  // namespace utils
}  // namespace dmp

#endif
