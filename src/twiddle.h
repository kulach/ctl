#pragma once

#include "common.h"
#include <vec.h>
#include <tview.h>
#include <cmath>
#include <numbers>


// The storage class of twiddles factors!
// While counterintuitive, we will store twiddle factors in this
// manner. Lets take the example of N = 8. We should therefore
// have 8 twiddle factors w0 -> w7. We can then store the factors 
// as such:
//
// [0]: 0 (Placeholder)
// [1]: w0 
// [2]: w0 w4
// [4]: w0 w2 w4 w6
// [8]: w0 w1 w2 w3 w4 w5 w6 w7
//
// This pattern allows easy access to contiguous data locations
// at the expense of more space taken up
template <typename T> requires FloatingType<T>
class TwiddleStore {
    public:
    using AlgType = complex<T>;
    static constexpr T PI = std::numbers::pi_v<T>;
    
    private:
    // Member is static so that holders may share this variable 
    static Vec<AlgType> hold;
    static size_t ref_count;

    void fill_last_layer(size_t n) {
        T* re = &hold.rdata()[n / 2];
        T* im = &hold.idata()[n / 2];

        for (size_t k = 0; k < n / 2; k++) {
            T angle = 2.0 * PI * (-1.0 * k) / (1.0 * n);
            re[k] = std::cos(angle); 
            im[k] = std::sin(angle); 
        }        
    }

    void fill_layer(size_t lsize) {
        T* re = &hold.rdata()[lsize / 2];
        T* im = &hold.idata()[lsize / 2];

        T* rsuper = &hold.rdata()[lsize];
        T* isuper = &hold.idata()[lsize];

        for (size_t i = 0; i < lsize / 2; i++) {
            re[i] = rsuper[2*i];
            im[i] = isuper[2*i];
        }
    }

    void fill_layers(size_t n) {
        fill_last_layer(n);
        for (size_t lsize = n / 2; lsize != 0; lsize /= 2) {
            fill_layer(lsize);
        }
    }

    void allocate(size_t n) {
        if (this->ref_count == 0 || n >= hold.size()) {
            this->hold = Vec<AlgType>{n};
            fill_layers(n);
        }
        this->ref_count++;
    }

    void deallocate() {
        this->ref_count--;
        if (this->ref_count == 0) {
            this->hold.~Vec();
        }
    }

    public:
    TwiddleStore(size_t n) {
        // Query for second half of Vec
        // Do operations on that
        ASSERT(util::is_pow2(n));
        allocate(n);
    }

    // Gets the layer with n twiddle factors
    // n must be a power of 2 and less than or equal to the precomputed size
    ConstView<complex<T>> get_layer(size_t n) const {
        ASSERT(util::is_pow2(n));
        return ConstView<complex<T>>(hold, n / 2, n / 2);
    }

    ~TwiddleStore() {
        deallocate();
    }
};


template <typename T> requires FloatingType<T>
size_t TwiddleStore<T>::ref_count = 0;

template <typename T> requires FloatingType<T>
Vec<complex<T>> TwiddleStore<T>::hold;
