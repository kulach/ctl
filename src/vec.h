#pragma once

#include <common.h>
#include <array>
#include <ranges>
#include <complex.h>
#include <arith.h>
#include <initializer_list>
#include <span>

template<typename T> requires ArithType<T>
class BaseNumVec {
    public:
    static const auto MAX_DIMS = 4;

    protected:
    T* _arr = nullptr;
    size_t _dim = 0;
    size_t _size = 0;
    std::array<uint32_t, MAX_DIMS> _dim_sizes;

    inline void calc_size() {
        size_t full_size = (_dim != 0) ? 1 : 0;
        for (size_t i = 0; i < _dim; i++) {
            full_size *= _dim_sizes[i];
        }
        _size = full_size;
    }

    inline void allocate(size_t n) {
        if (_size != 0) {
            size_t alloced = util::ceil_align<arith<T>::Alignment>(n);
            _arr = reinterpret_cast<T*>(std::aligned_alloc(arith<T>::Alignment, sizeof(T) * alloced));
        }
    }

    inline void clean() {
        if (_arr) {
            free(_arr);
        }
        _arr = nullptr;
        _dim = 0;
        _size = 0;
    }

    inline void copy(const BaseNumVec& other) {
        this->_dim = other._dim;
        this->_size = other._size;
        this->_dim_sizes = other._dim_sizes;
        allocate(this->_size);
        // We are able to copy simply because we are using scalar types, ie types lacking
        // constructors, move semantics, memory management, io, etc
        std::memcpy(this->_arr, other._arr, sizeof(T) * _size);
    }

    inline void move(BaseNumVec&& other) noexcept {
        this->_arr = other._arr;
        other._arr = nullptr;
        this->_dim = other._dim;
        other._dim = 0;
        this->_size = other._size;
        other._size = 0;
        this->_dim_sizes = other._dim_sizes;
    }

    public:

    BaseNumVec() : _arr(nullptr), _size(0) {
    }

    BaseNumVec(std::initializer_list<size_t> init) 
        : _arr(nullptr), _dim(init.size()) {
        auto it = init.begin();
        for (size_t i = 0; i < _dim; i++) {
            _dim_sizes[i] = *it;
            ++it; 
        }
        calc_size();
        allocate(this->_size);     
    }

    
    template <typename Range> requires std::ranges::input_range<Range>
    BaseNumVec(const Range& range) : _arr(nullptr), _dim(0) {
        size_t i = 0;
        for (auto d : range) {
            ASSERT(i < MAX_DIMS);
            if (d == 0) break; // Allows a sentinel value of 0
            _dim_sizes[i] = d;
            i++;
        }
        _dim = i;
        calc_size();
        allocate(this->_size);
    }

    inline std::span<const uint32_t> _dims() const {
        const auto s = std::span<const uint32_t>(_dim_sizes.cbegin(), _dim);
        // const std::span<uint32_t> s(_dim_sizes.cbegin(), _dim);
        return s;
    }

    BaseNumVec(const BaseNumVec& other) {
        copy(other);
    }

    BaseNumVec& operator=(const BaseNumVec& other) {
        clean();
        copy(other);
        return *this;
    }

    BaseNumVec(BaseNumVec&& other) noexcept {
        move(std::move(other));
    }

    BaseNumVec& operator=(BaseNumVec&& other) {
        clean();
        move(std::move(other));
        return *this;
    }

    inline size_t ndim() const noexcept {
        return _dim;
    }

    // Distance between indices on first dimension
    inline size_t stride() const noexcept {
        return _size / _dim_sizes[0];
    }

    inline bool empty() const noexcept {
        return _size == 0;
    }

    inline T* data() const noexcept {
        return _arr;
    }

    inline void zero() const noexcept {
        for (size_t i = 0; i < _size; i++) {
            // Don't use memset since types have opportunity of being strange
            _arr[i] = 0;
        }
    }

    ~BaseNumVec() {
        clean();
    }

};

template <typename T> requires ArithType<T>
class Vec : public BaseNumVec<T> {
    using tarith = arith<T>;
    using PtrType = T*;
    public:

    Vec(std::initializer_list<size_t> init) : 
        BaseNumVec<T>(init) {
    }

    Vec() : BaseNumVec<T>() {
    }

    template<typename Range> requires std::ranges::input_range<Range>
    Vec(Range&& range) : BaseNumVec<T>(std::forward<Range>(range)) {
    }

    inline PtrType data_ptr() {
        return this->data();
    }

    inline size_t size() const noexcept {
        return this->_size;
    }

};

template<typename T> requires ComplexType<complex<T>>
class Vec<complex<T>> : public BaseNumVec<T> {
    using tarith = carith<complex<T>>;
    public:

    static constexpr auto BASE_MAX_DIMS = BaseNumVec<complex<T>>::MAX_DIMS;
    static constexpr auto MAX_DIMS = BASE_MAX_DIMS - 1;

    using PtrType = complexptr<T>;
    using ConstPtrType = ccomplexptr<T>;

    template <typename Range>
    BaseNumVec<T> generate_base(Range& range) {
        std::array<uint32_t, BASE_MAX_DIMS> dim_sizes;
        uint32_t dims = 1;
        dim_sizes[0] = 2; // One for re, one for im
        for (auto r : range) {
            size_t dim = (dims == 1) ? util::ceil_align<tarith::OpCapacity>(r) : r;
            dim_sizes[dims] = dim;
            dims++;
        }

        auto span = std::span<uint32_t>(dim_sizes.begin(), dims);
        return BaseNumVec<T>(span);
    }

    public:
    Vec(std::initializer_list<size_t> init) : BaseNumVec<T>(generate_base(init)) {
        // Use default constructor, then move new one into it
        std::array<size_t, MAX_DIMS> base_dims;

        base_dims[0] = 2; // One dimension for real, one for imaginary
        uint32_t i = 1;
        // TODO -- multiply subsequent dimensions to find out 
        // the correct aligment for this
        for (auto d : init) {
            ASSERT(i < MAX_DIMS);
            base_dims[i] = d;
            i++;
        }
    }

    inline Vec() {
    };

    inline T* rdata() noexcept {
        return &this->_arr[0];
    }

    inline const T* rdata() const noexcept {
        return &this->_arr[0];
    }

    inline T* idata() noexcept {
        return &this->_arr[this->_size / 2];
    }

    inline const T* idata() const noexcept {
        return &this->_arr[this->_size / 2];
    }

    inline PtrType data_ptr() noexcept {
        return PtrType{rdata(), idata()};
    }

    // The number of complex numbers in the container
    inline size_t size() const noexcept {
        return this->_size / 2;
    }

    // Stride between re and im components
    inline size_t comp_stride() const noexcept {
        return size();
    }

    // Distance between two elements on the array
    // In this case, it is the size of the second dimension
    inline size_t stride() const noexcept {
        return this->size() / this->_dim_sizes[1];
    }

};
