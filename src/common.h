#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <utility>
#include <type_traits>
#include <cstring>
#include <iostream>

#ifdef BUILD_DEBUG
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "Assertion failed: (" #condition "), function " << __FUNCTION__ \
                      << ", file " << __FILE__ << ", line " << __LINE__ << "." << std::endl; \
            std::abort(); \
        } \
    } while (false)
#else

#define ASSERT(condition) do {} while (false)

#endif

template<typename T>
concept ScalarType = std::is_scalar_v<T>;

template<typename T>
concept FloatingType = std::is_floating_point_v<T>;

template<typename T>
concept IntegerType = std::is_integral_v<T>;


template <typename T>
concept ComplexType = requires(T type) {
    typename T::BaseType;
    requires FloatingType<typename T::BaseType>;
    {type.re} noexcept;
    {type.im} noexcept;
    requires std::same_as<std::remove_reference_t<decltype(type.re)>, typename T::BaseType>;
    requires std::same_as<std::remove_reference_t<decltype(type.im)>, typename T::BaseType>;
};

template<typename T>
concept ArithType = (ScalarType<T> || ComplexType<T>);

template <typename View>
concept VecViewType = requires(View v, size_t i) {
    {v[i]} -> std::same_as<typename View::RefType>;
    {v.data()} -> std::same_as<typename View::PtrType>;
};

namespace util {
    
    // Gives you the maximum size allowed if following 
    // alignment requirements
    // 3 align 4 -> 4
    // 4 align 4 -> 4
    // 5 align 4 -> 8
    // etc.
    template<size_t ALIGNMENT>
    constexpr size_t ceil_align(size_t size) noexcept {
        return ((size - 1) / ALIGNMENT + 1) * ALIGNMENT;
    }

    template<typename T>
    constexpr bool is_pow2(T a) noexcept {
        return (a != 0) && ((a & (a >> 1)) == 0);
    }

    constexpr uint64_t pow2(uint64_t a) noexcept {
        return 1 << a;
    }
}
