#pragma once

#include <tview.h>
#include <arith.h>

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline MutType& operator+=(MutType& a, const ConstType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_add_vec(a.data(), a.data(), b.data(), a.size());
    return a;
}

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline MutType& operator-=(MutType& a, const ConstType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_sub_vec(a.data(), a.data(), b.data(), a.size());
    return a;
}

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline MutType& operator*=(MutType& a, const ConstType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_mul_vec(a.data(), a.data(), b.data(), a.size());
    return a;
}

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline MutType& operator/=(MutType& a, const ConstType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_div_vec(a.data(), a.data(), b.data(), a.size());
    return a;
}

template<typename MutType> requires VecViewType<MutType>
inline MutType& operator+=(MutType& a, const typename MutType::AlgType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_add_scalar(a.data(), a.data(), b, a.size());
    return a;
}

template<typename MutType> requires VecViewType<MutType>
inline MutType& operator*=(MutType& a, const typename MutType::AlgType& b) noexcept {
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_mul_scalar(a.data(), a.data(), b, a.size());
    return a;
}

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline void altAddSubProd(MutType& outa, MutType& outb, const ConstType& c) noexcept { 
    ASSERT(outa.size() == outb.size());
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_faltmaddsub_vec(outa.data(), outb.data(), outa.data(), outb.data(), c.data(), outa.size());
}

template<typename MutType, typename ConstType> requires VecViewType<MutType> && VecViewType<ConstType>
inline void altAddSubMultConj(MutType& outa, MutType& outb, const ConstType& c) noexcept { 
    ASSERT(outa.size() == outb.size());
    using tarith = Arith<typename MutType::AlgType>;
    tarith::_faltaddsubmultconj(outa.data(), outb.data(), outa.data(), outb.data(), c.data(), outa.size());
}
