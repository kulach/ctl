#pragma once

#include <common.h>

template <typename T> requires FloatingType<T>
struct complex {
    using BaseType = T;
    T re;
    T im;

    complex(const T& re, const T& im) : re(re), im(im) {}
    complex() {}

    friend std::ostream& operator<<(std::ostream& os, const complex& ref) {
        os << "[" << ref.re << "," << ref.im << "]";
        return os;
    }
};

template <typename Ref>
concept ComplexRefType = requires(Ref ref, size_t i, typename Ref::BaseType base) {
    {Ref(base, base)} -> std::same_as<Ref>;
    requires std::same_as<std::remove_cvref_t<decltype(ref.re)>, typename Ref::BaseType>;
    requires std::same_as<std::remove_cvref_t<decltype(ref.im)>, typename Ref::BaseType>;
};

template <typename T> requires FloatingType<T>
struct complexref {
    using BaseType = T;
    T& re;
    T& im;

    complexref(T& re, T& im) noexcept : re(re), im(im) {}

    complexref(complex<T>& orig) noexcept : re(orig.re), im(orig.im) {}

    complexref(const complexref& other) noexcept : re(other.re), im(other.im) {}

    complexref(complexref&& other) noexcept : re(other.re), im(other.im) {}

    template <typename Ref> requires ComplexRefType<Ref>
    complexref& operator=(const Ref& other) noexcept {
        re = other.re;
        im = other.im;
        return *this;
    }

    template <typename Ref> requires ComplexRefType<Ref>
    complexref& operator=(Ref&& other) noexcept {
        re = other.re;
        im = other.im;
        return *this;
    }

    template <typename Ref> requires ComplexRefType<Ref>
    complexref& operator=(const Ref& other) const noexcept {
        re = other.re;
        im = other.im;
        return *this;
    }

    friend void swap(complexref& a, complexref& b) noexcept {
        std::swap(a.re, b.re);
        std::swap(a.im, b.im);
    }

    friend void swap(complexref&& a, complexref&& b) noexcept {
        std::swap(a.re, b.re);
        std::swap(a.im, b.im);
    }

    friend std::ostream& operator<<(std::ostream& os, const complexref& ref) {
        os << "[" << ref.re << "," << ref.im << "]";
        return os;
    }
};

template <typename T> requires FloatingType<T>
struct ccomplexref {
    using BaseType = T;
    const T& re;
    const T& im;

    ccomplexref(const T& re, const T& im) noexcept : re(re), im(im) {}

    ccomplexref(const complex<T>& orig) noexcept : re(orig.re), im(orig.im) {}

    ccomplexref(const complexref<T>& orig) noexcept : re(orig.re), im(orig.im) {}

    friend std::ostream& operator<<(std::ostream& os, const ccomplexref& ref) {
        os << "[" << ref.re << "," << ref.im << "]";
        return os;
    }

};

template <typename Ptr>
concept ComplexPtrType = requires(Ptr ptr, size_t i, typename Ptr::BaseType* base) {
    {ptr[i]} -> std::same_as<typename Ptr::RefType>;
    {*ptr} -> std::same_as<typename Ptr::RefType>;
    {Ptr(nullptr)} -> std::same_as<Ptr>;
    {Ptr(base, base)} -> std::same_as<Ptr>;
    {ptr = ptr} noexcept;
    {ptr = std::move(ptr)} noexcept;
    requires std::same_as<std::remove_reference_t<decltype(ptr.re)>, typename Ptr::BaseType*>;
    requires std::same_as<std::remove_reference_t<decltype(ptr.im)>, typename Ptr::BaseType*>;
};

// Pointer arithmetic operations
template <typename Ptr> requires ComplexPtrType<Ptr>
inline Ptr operator+(const Ptr& ptr, ptrdiff_t offset) noexcept {
    return Ptr{ptr.re + offset, ptr.im + offset};
}

template <typename Ptr> requires ComplexPtrType<Ptr>
inline Ptr operator-(const Ptr& ptr, ptrdiff_t offset) noexcept {
    return Ptr{ptr.re - offset, ptr.im - offset};
}

template <typename Ptr> requires ComplexPtrType<Ptr>
inline Ptr& operator+=(Ptr& ptr, ptrdiff_t offset) noexcept {
    ptr.re += offset;
    ptr.im += offset;
    return ptr;
}

template <typename Ptr> requires ComplexPtrType<Ptr>
inline Ptr& operator-=(Ptr& ptr, ptrdiff_t offset) noexcept {
    ptr.re -= offset;
    ptr.im -= offset;
    return ptr;
}

template <typename Ptr> requires ComplexPtrType<Ptr>
inline bool operator==(const Ptr& lhs, const Ptr& rhs) noexcept {
    // Assert that real pointer equality imples im pointer equality
    // and vice versa
    ASSERT((lhs.re == rhs.re) == (lhs.im == rhs.im));
    return lhs.re == rhs.re;
}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr bool operator!=(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re != rhs.re;}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr bool operator<(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re < rhs.re;}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr bool operator>(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re > rhs.re;}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr bool operator<=(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re <= rhs.re;}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr bool operator>=(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re >= rhs.re;}

template<typename Ptr> requires ComplexPtrType<Ptr>
constexpr ptrdiff_t operator-(const Ptr& lhs, const Ptr& rhs) noexcept {return lhs.re - rhs.re;}

// template <typename Ptr> requires ComplexPtrType<Ptr>
// inline Ptr::RefType operator*(const Ptr& ptr) noexcept {
//     return Ptr::RefType{*(ptr.re), *(ptr.im)};
// }

// inline complexref<T> operator[](size_t index) const noexcept {
//     return {re[index], im[index]};
// }
template<typename T> requires FloatingType<T>
struct complexptr {
    using BaseType = T;
    using RefType = complexref<BaseType>;
    T* re;
    T* im;
    complexptr(T* re, T* im) : re(re), im(im) {}

    // Constructor required for complexptr(nullptr) to evaluate correctly
    complexptr(T*) : re(nullptr), im(nullptr) {}

    complexptr(const complexptr& other) : re(other.re), im(other.im) {}
    complexptr(complexptr&& other) : re(other.re), im(other.im) {}

    complexptr& operator=(const complexptr& other) noexcept {
        re = other.re;
        im = other.im;
        return *this;
    }

    complexptr& operator=(complexptr&& other) noexcept {
        re = other.re;
        im = other.im;
        return *this;
    }

    inline complexref<T> operator*() const noexcept {
        return complexref<T>{*re, *im};
    }

    inline complexref<T> operator[](size_t index) const noexcept {
        return {re[index], im[index]};
    }
};

// Const complex ref
template<typename T> requires FloatingType<T>
struct ccomplexptr {
    using BaseType = T;
    const T* re;
    const T* im;
    ccomplexptr(const T* re, const T* im) : re(re), im(im) {}

    ccomplexptr(const complexptr<T> orig) : re(orig.re), im(orig.im){}

    ccomplexptr(T*) : re(nullptr), im(nullptr) {}

    inline ccomplexref<T> operator*() const noexcept {
        return ccomplexref<T>{*re, *im};
    }

    inline ccomplexref<T> operator[](size_t index) const noexcept {
        return {re[index], im[index]};
    }
};

using cplx64_t = complex<float>;
using cplx128_t = complex<double>;
