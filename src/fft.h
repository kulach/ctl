#pragma once

#include "operation.h"
#include "tview.h"
#include <common.h>
#include <function.h>
#include <shuffler.h>
#include <twiddle.h>

template <typename T> requires ScalarType<T>
class FFT : public BaseFunction<complex<T>> {

    public:
    using BaseType = T;
    using AlgType = complex<T>;

    bool forward = true;
    private:
    ShuffleFunction<complex<T>> shuffler;
    const TwiddleStore<T> twiddles;

    // Specific implementation for FFTs of size 2 (1 even 1 odd)
    void _fft_layer_2_impl(MutView<AlgType>& even, MutView<AlgType>& odd) const noexcept {  
        auto r0 = *even.data().re;
        auto i0 = *even.data().im;

        *even.data().re = r0 + *odd.data().re;
        *even.data().im = i0 + *odd.data().im;
        *odd.data().re = r0 - *odd.data().re;
        *odd.data().im = i0 - *odd.data().im;
    }

    void _ifft_layer_2_impl(MutView<AlgType>& even, MutView<AlgType>& odd) const noexcept {
        // Unsurprisingly, these are the same
        _fft_layer_2_impl(even, odd);
    }

    // Specific implementation of FFTs of size 4 (2 even 2 odd)
    void _fft_layer_4_impl(MutView<AlgType>& even, MutView<AlgType>& odd) const noexcept {
        BaseType rt, it; // Real tmp Imag tmp
        
        rt = even.data().re[0];
        it = even.data().im[0];
        // W = 1
        even.data().re[0] = rt + odd.data().re[0];
        even.data().im[0] = it + odd.data().im[0];
        // W = -1
        odd.data().re[0] = rt - odd.data().re[0];
        odd.data().im[0] = it - odd.data().im[0];

        rt = even.data().re[1];
        it = even.data().im[1];

        // Twiddle factor will be -j and j
        // W = -j
        even.data().re[1] = rt + odd.data().im[1];
        even.data().im[1] = it - odd.data().re[1];
        // W = j 
        // rt filled with tmp value here o_1
        rt = rt - odd.data().im[1];
        odd.data().im[1] = it + odd.data().re[1];
        odd.data().re[1] = rt; // Place it back 
    }

    void _ifft_layer_4_impl(MutView<AlgType>& even, MutView<AlgType>& odd) const noexcept {
        BaseType rt, it;
        rt = even.data().re[0];
        it = even.data().im[0];

        // E = E + O
        even.data().re[0] = even.data().re[0] + odd.data().re[0];
        even.data().im[0] = even.data().im[0] + odd.data().im[0];

        // E = (E - O) * W*_[0,N/2) ... W_0 = 1, W_N/2 = -j
        odd.data().re[0] = rt - odd.data().re[0];
        odd.data().im[0] = it - odd.data().im[0];

        rt = even.data().re[1];
        it = even.data().im[1];

        // E = E + O
        even.data().re[1] = even.data().re[1] + odd.data().re[1];
        even.data().im[1] = even.data().im[1] + odd.data().im[1];

        // E = (E - O) * W*_[0,N/2) ... W_0 = 1, W_N/2 = -j
        odd.data().re[1] = rt - odd.data().re[1];
        odd.data().im[1] = -(it - odd.data().im[1]);
        std::swap(odd.data().re[1], odd.data().im[1]);

    }

    inline void _ifft_layer_n_impl(MutView<AlgType>& even, MutView<AlgType>& odd) const noexcept {
        // E = E + O 
        // O = (E - O) * W*_[0,N/2)

        // Described less nicely:
        // Input to FFT Layer -> Output of FFT Layer -> Output of IFFT Layer (this)
        // E -> E + W_[0,N/2) * O -> E + W_[0,N/2) * O + E - W_[0,N/2) * O = 2*E
        // O -> E - W_[0,N/2) * O -> (E + W_[0,N/2) * O - E + W_[0,N/2) * O) * W*_[0,N/2) = 2 * W_[0,N/2) * O * W*_[0,N/2) * O =  2 * O

        const auto& twid = twiddles.get_layer(2*even.size());
        altAddSubMultConj(even, odd, twid);
    }

    // First half of the input slice is the even FT 
    // Second half of the slice is the odd FT
    void _fft_layer_n_impl(MutView<AlgType>& even, MutView<AlgType>& odd, ConstView<AlgType>& twid) const noexcept {
        // Goal is to create F = E + W*O
        // Easier said than done -- Even and Odd sides are only half the final output size
        
        // E = E + W_[0,N/2) * O
        // O = E - W_[0,N/2) * O

        // We only use half of the available twiddle values in this calculation
        // Therefore, we do not need to calculate half of calculated twiddles
        altAddSubProd(even, odd, twid);
    }

    inline void _fft_layer_impl(MutView<AlgType>& layer, size_t batch_size) const noexcept {
        ASSERT(batch_size > 1);
        int64_t niter = shuffler.size() / batch_size;
        auto twid = twiddles.get_layer(batch_size);
        for (int64_t i = 0; i < niter; i++) {
            int64_t offset = batch_size * i;
            MutView<AlgType> even(layer.data() + offset, batch_size / 2);
            MutView<AlgType> odd(layer.data() + (offset + batch_size / 2), batch_size / 2);
            if (batch_size == 2) {
                _fft_layer_2_impl(even, odd);
            } else if (batch_size == 4) {
                _fft_layer_4_impl(even, odd);
            } else {
                _fft_layer_n_impl(even, odd, twid);
            }
        }
    }

    inline void _ifft_layer_impl(MutView<AlgType>& layer, size_t batch_size) const noexcept {
        ASSERT(batch_size > 1);
        int64_t niter = shuffler.size() / batch_size;
        for (int64_t i = 0; i < niter; i++) {
            int64_t offset = batch_size * i;
            MutView<AlgType> even(layer.data() + offset, batch_size / 2);
            MutView<AlgType> odd(layer.data() + (offset + batch_size / 2), batch_size / 2);
            if (batch_size == 2) {
                _ifft_layer_2_impl(even, odd);
            } else if (batch_size == 4) {
                _ifft_layer_4_impl(even, odd);
            } else {
                _ifft_layer_n_impl(even, odd);
            }
        }
    }

    // The trivial algorithm
    // This will be used as a base algorithm to test other finer-tuned
    // implementations that allow for better caching of data
    void _fft_impl(MutView<AlgType>& data) const noexcept {
        for (size_t batch_size = 2; batch_size <= data.size(); batch_size *= 2) {
            _fft_layer_impl(data, batch_size);
        }
    }

    void _ifft_impl(MutView<AlgType>& data) const noexcept {
        for (size_t batch_size = shuffler.size(); batch_size >= 2; batch_size /= 2) {
            _ifft_layer_impl(data, batch_size);
        }

        AlgType mult{1.0 / (1.0 * shuffler.size()), 0.0};
        data *= mult;
    }

    public:
    FFT(size_t N, bool forward = true) : forward(forward), shuffler(N), twiddles(N) {
        ASSERT(util::is_pow2(N)); 
    }

    MutView<AlgType> fft(MutView<AlgType> input) const {
        input = shuffler(input);
        _fft_impl(input);

        return input;
    }

    MutView<AlgType> ifft(MutView<AlgType> input) const {
        _ifft_impl(input);
        input = shuffler(input);
  
        return input;
    }

    MutView<AlgType> operator()(MutView<AlgType> input) const override {
        if (forward) return fft(std::move(input));
        else return ifft(std::move(input));
    }

    size_t input_size() const override {
        return 1;
    }

    inline size_t size() noexcept {
        return shuffler.size();
    }
};

template class FFT<double>;
