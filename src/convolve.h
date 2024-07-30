#pragma once

#include <function.h>
#include <fft.h>

// Although convolution does not require only complex types
// our FFT algorithm only supports complex values, therefore
// we will not use them
template<typename T> requires ComplexType<T>
class ConvolutionFunction : public BaseFunction<T> {
    FFT<typename T::BaseType> fft;
    Vec<T> kernel;
    MutView<T> kview;
    public:

    ConvolutionFunction(Vec<T> kernel) : 
        fft(kernel.size()), kernel(std::move(kernel)), kview(this->kernel) {
        dft_kernel();
    }

    MutView<T> operator()(MutView<T> input) const override {
        input = fft.fft(input);
        input *= kview;
        input = fft.ifft(input);
        return input; // So easy
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    void dft_kernel() {
        fft.fft(kview);
    }
};
