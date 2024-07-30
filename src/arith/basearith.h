#pragma once

#include <common.h>

template <typename Op, typename Intrin>
concept VecOp = requires(Op, Intrin& c, const Intrin& a, const Intrin& b) {
    {Op::_exec(c, a, b)} noexcept;
};

namespace autil {
    template <size_t CAPACITY>
    static inline size_t _num_loops(size_t n) noexcept {
        return ((n - 1) / CAPACITY + 1);
    }
    
    template<size_t ALIGNMENT, typename T>
    static inline bool _assert_align_impl(const T* ptr) noexcept {
        return (reinterpret_cast<uint64_t>(ptr) % ALIGNMENT == 0);
    }

    template<size_t ALIGNMENT, typename... Args>
    static inline bool _assert_align(Args&&... args) noexcept {
        return (true && ... && _assert_align_impl<ALIGNMENT>(args));
    }
};
