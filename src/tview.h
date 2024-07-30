#pragma once

#include <common.h>
#include <iterator>
#include <vec.h>
#include <operation.h>
#include <type_traits>

namespace tview {
    template <typename T>
    inline T* calc_ptr(const Vec<T>& Vec, size_t index) noexcept {
        return &Vec.data()[index * Vec.stride()];
    }

    template <typename T>
    inline complexptr<T> calc_ptr(Vec<complex<T>>& Vec, size_t index) noexcept {
        auto stride = Vec.stride();
        T* re = &Vec.rdata()[index * stride];
        T* im = &Vec.idata()[index * stride];
        return {re, im};
    }

    template <typename T>
    inline ccomplexptr<T> calc_ptr(const Vec<complex<T>>& Vec, size_t index) noexcept {
        auto stride = Vec.stride();
        const T* re = &Vec.rdata()[index * stride];
        const T* im = &Vec.idata()[index * stride];
        return {re, im};
    }

    // Implementation of a random_access_iterator
    // Random access required for std::algorithm functions,
    // and it is just the most convenient way to use iterators
    // (although not the easiest to implement)
    template<typename ViewImpl> 
    class _VecViewIterator {
        public:
        using PtrType = ViewImpl::PtrType;
        using RefType = ViewImpl::RefType;

        using difference_type = ptrdiff_t;
        using value_type = typename std::remove_reference_t<RefType>;
        using iterator_category = typename std::random_access_iterator_tag;

        private:
        PtrType _ptr;
        public:

        _VecViewIterator(PtrType ptr) : _ptr(std::move(ptr)) {}

        _VecViewIterator() : _ptr(nullptr) {}

        constexpr _VecViewIterator& operator+=(difference_type delta) noexcept {
            _ptr += delta;
            return *this;
        }

        constexpr _VecViewIterator& operator-=(difference_type delta) noexcept {
            _ptr -= delta;
            return *this;
        }

        // Prefix operator
        constexpr _VecViewIterator& operator++() noexcept {
            return this->operator+=(1);
        }

        constexpr _VecViewIterator& operator--() noexcept {
            return this->operator-=(1);
        }

        // Postfix operator (dummy variable included)
        // Don't use this if you appreciate your sanity
        constexpr _VecViewIterator operator++(int) noexcept {
            auto tmp = *this;
            this->operator++();
            return tmp;
        }

        constexpr _VecViewIterator operator--(int) noexcept {
            auto tmp = *this;
            this->operator--();
            return tmp;
        }

        constexpr _VecViewIterator operator+(difference_type rhs) const noexcept {return _VecViewIterator(_ptr + rhs);}
        constexpr _VecViewIterator operator-(difference_type rhs) const noexcept {return _VecViewIterator(_ptr - rhs);}
        constexpr difference_type operator-(const _VecViewIterator& other) const noexcept {return this->_ptr - other._ptr;}
        friend constexpr _VecViewIterator operator+(difference_type delta, const _VecViewIterator& rhs) noexcept {return _VecViewIterator(rhs._ptr + delta);}
        friend constexpr _VecViewIterator operator-(difference_type delta, const _VecViewIterator& rhs) noexcept {return _VecViewIterator(rhs._ptr - delta);}

        // constexpr operator PtrType() const noexcept {return _ptr;}
        // constexpr operator RefType() const noexcept {return *_ptr;}
        constexpr RefType operator*() const noexcept {return *_ptr;}
        constexpr PtrType operator->() const noexcept {return _ptr;}
        constexpr RefType operator[](difference_type offset) const noexcept {return _ptr[offset];}
        constexpr bool operator==(const _VecViewIterator& other) const noexcept {return this->_ptr == other._ptr;}
        constexpr bool operator!=(const _VecViewIterator& other) const noexcept {return this->_ptr != other._ptr;}
        constexpr bool operator<(const _VecViewIterator& other) const noexcept {return this->_ptr < other._ptr;}
        constexpr bool operator>(const _VecViewIterator& other) const noexcept {return this->_ptr > other._ptr;}
        constexpr bool operator<=(const _VecViewIterator& other) const noexcept {return this->_ptr <= other._ptr;}
        constexpr bool operator>=(const _VecViewIterator& other) const noexcept {return this->_ptr >= other._ptr;}
    };

    template<typename _AlgType, typename _RefType, typename _PtrType, typename _VecType>
    class _VecViewImpl {
        public:
        using AlgType = _AlgType;
        using RefType = _RefType;
        using PtrType = _PtrType;
        using VecType = _VecType;
        using iterator = _VecViewIterator<_VecViewImpl>;

        private:
        PtrType _arr;
        size_t _size;

        public:
        _VecViewImpl(VecType& Vec, size_t index, size_t n) 
            : _arr(tview::calc_ptr(Vec, index)), _size(n) {
            }

        _VecViewImpl(VecType& Vec) : _VecViewImpl(Vec, 0, Vec.size()) {}

        _VecViewImpl(PtrType arr, size_t size) : _arr(arr), _size(size) {}

        inline PtrType data() const noexcept {
            return _arr;
        }

        // Only works for vector sized Vecs TODO
        inline std::remove_const_t<VecType> make_copy() const noexcept {
            std::remove_const_t<VecType> t{_size};
            auto data = t.data_ptr();
            for (size_t i = 0; i < _size; i++) {
                 data[i] = _arr[i];
            }
            return t;
        }

        inline RefType operator[](size_t index) const noexcept {
            return _arr[index];
        }

        inline size_t size() const noexcept {
            return _size;
        }

        inline iterator begin() const noexcept {
            return iterator(_arr);
        };

        inline iterator end() const noexcept {
            auto ptr = _arr + _size;
            return iterator(ptr);
        }

    };
} // namespace tview

template<typename T> requires ArithType<T>
struct ConstViewImpl {
    using Type = tview::_VecViewImpl<T, const T&, const T*, const Vec<T>>;
};

template<typename T> requires ComplexType<complex<T>>
struct ConstViewImpl<complex<T>> {
    using Type = tview::_VecViewImpl<const complex<T>, ccomplexref<T>, ccomplexptr<T>, const Vec<complex<T>>>;
};

template<typename T> requires ArithType<T>
struct MutViewImpl {
    using Type = tview::_VecViewImpl<T, T&, T*, Vec<T>>;
};

template<typename T> requires ComplexType<complex<T>>
struct MutViewImpl<complex<T>> {
    using Type = tview::_VecViewImpl<complex<T>, complexref<T>, complexptr<T>, Vec<complex<T>>>;
};

template <typename T>
using ConstView = ConstViewImpl<T>::Type;

template <typename T>
using MutView = MutViewImpl<T>::Type;

// Random access iterators required to use std::algorithm functionality correctly
static_assert(std::random_access_iterator<MutView<complex<double>>::iterator>);
static_assert(std::random_access_iterator<MutView<double>::iterator>);
// Permutable iterators necessary for std::rotate
static_assert(std::permutable<MutView<double>::iterator>);
static_assert(std::permutable<MutView<complex<double>>::iterator>);

namespace tview {
    template <typename T> requires ArithType<T>
    MutView<T> view(Vec<T>& Vec) {
        return MutView<T>(Vec);
    }
}
