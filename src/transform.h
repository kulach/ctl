#pragma once

#include "common.h"
#include <function.h>
#include <fft.h>
#include <complex.h>
#include <convolve.h>

template <typename T> requires FloatingType<T>
class FourierDual {
    // Overall layout of this class
    // We will have a forward Transform
    // and a backwards transform

    // Given that we first can multiply the input by a scalar
    // The resulting output is the same output as before, with that scalar
    // This class will therefore be giving you the tools to 
    // view the output of a modified input given the output 
    // of the original image

    // Ex: Given F(k) <-> f(x)
    //       v1(F(k)) <-> u1(f(x))
    // and   v2(F(k)) <-> u2(f(x)) ...

    // We then will know that... 
    //
    //           F(k) <-> f(x)      
    //           G(k) <-> g(x)
    //           g(x) = u2(u1(f(x)))  (Given)
    //
    // We can logically conclude that...
    //
    //           G(k) = v2(v1(F(k)))
    //
    // Which means that we can perform a myriad of 
    // functional transforms to the input, as long as we
    // create a bespoke (u, v) function pair for all known
    // properties

    public:
    // Composte functions must be using the complex algebra space
    // Pinning them to real functions requires
    // moer specific properties
    using AlgType = complex<T>;
    using Func = BaseFunction<complex<T>>;
    using FuncPtr = std::shared_ptr<BaseFunction<complex<T>>>;
    using CompFunc = CompositeFunction<complex<T>>;

    private:

    CompFunc time_func; // Same as u(k)
    CompFunc freq_func; // Same as v(k)

    size_t _size;
                        
    public:

    FourierDual(size_t n) : _size(n) {}

    // Setting v(k)
    void set_time_func(const CompFunc& func) {
        clear_functions();

        auto& funcs = func.get_funcs();
        for (auto& func : funcs) {
            compose_time(func);
        }
    }

    void set_freq_func(const CompFunc& func) {
        clear_functions();

        auto& funcs = func.get_funcs();
        for (auto& func : funcs) {
            compose_freq(func);
        }
    }

    MutView<AlgType> u(MutView<AlgType> input) {
        return time_func(input);
    }

    MutView<AlgType> v(MutView<AlgType> input) {
        return freq_func(input);
    }

    private:
    void clear_functions() {
        time_func = CompFunc();
        freq_func = CompFunc();
    }

    FuncPtr transform_time_func(const ScalerFunction<AlgType>& u) {
        // Scaling is trivial -- no need to create a new function if it is exactly the same
        return std::make_shared<ScalerFunction<AlgType>>(u.scale());
    }

    FuncPtr transform_time_func(const SumFunction<AlgType>& u) {
        auto addend_time = u.get_addend();
        FFT<T> fft(addend_time.size());
        auto data = addend_time.make_copy();

        fft(tview::view(data));

        return std::make_shared<SumFunction<complex<T>>>(data);
    }

    FuncPtr transform_time_func(const ShiftFunction<AlgType>& u) {
        // Multiply by exp(-2*pi*j*a / N * k)
        // auto mult_freq = Vec<complex<T>>k;
        Vec<complex<T>> rots{_size};
        auto rview = tview::view(rots);

        int32_t i = 0;
        for (auto r : rview) {
            double angle = -2.0 * std::numbers::pi * (1.0 * u.shift()) * (1.0 * i++) / (1.0 * _size);
            r = complex<double>{std::cos(angle), std::sin(angle)};
        }
        return std::make_shared<MultFunction<AlgType>>(rots);
    }
    
    FuncPtr transform_time_func(const ConjFunction<AlgType>&) {
        auto comp = std::make_shared<CompositeFunction<AlgType>>();
        comp->compose_outer(std::make_shared<CircularReverse<AlgType>>());
        comp->compose_outer(std::make_shared<ConjFunction<AlgType>>());
        return comp;
    }

    FuncPtr transform_time_func(const ProductFunction<AlgType>& u) {
        // Vec<complex<T>> prods{_size};
        auto comp = std::make_shared<CompositeFunction<AlgType>>();

        auto kernel = u.get_multiplicand().make_copy();

        FFT<T> fft(kernel.size());
        fft.fft(kernel);

        auto conv = std::make_shared<ConvolutionFunction<AlgType>>(kernel);
        
        comp->compose_outer(conv);

        comp->compose_outer(std::make_shared<ScalerFunction<AlgType>>(AlgType{1.0 / (1.0 * _size), 0.0}));

        return comp;
    }

    FuncPtr transform_freq_func(const ScalerFunction<AlgType>& v) {
        return std::make_shared<ScalerFunction<AlgType>>(v.scale());
    }

    FuncPtr transform_freq_func(const SumFunction<AlgType>& v) {
        auto addend_freq = v.get_addend();
        FFT<T> fft(addend_freq.size());
        auto data = addend_freq.make_copy();

        fft.ifft(tview::view(data));
        return std::make_shared<SumFunction<complex<T>>>(data);
    }

    FuncPtr transform_freq_func(const ShiftFunction<AlgType>& v) {
        // Multiply by exp(-2*pi*j*a / N * k)
        // auto mult_freq = Vec<complex<T>>k;
        Vec<complex<T>> rots{_size};
        auto rview = tview::view(rots);

        int32_t i = 0;
        for (auto r : rview) {
            double angle = 2.0 * std::numbers::pi * (1.0 * v.shift()) * (1.0 * i++) / (1.0 * _size);
            r = complex<double>{std::cos(angle), std::sin(angle)};
        }
        return std::make_shared<MultFunction<AlgType>>(rots);
    }

    FuncPtr transform_freq_func(const ProductFunction<AlgType>& v) {
        auto comp = std::make_shared<CompositeFunction<AlgType>>();

        auto kernel = v.get_multiplicand().make_copy();

        // We need an ifft with a operator() default...
        auto ifft = FFT<T>(kernel.size());

        ifft.ifft(kernel);

        auto conv = std::make_shared<ConvolutionFunction<AlgType>>(kernel);

        comp->compose_outer(conv);
        return comp;
    }

    void compose_freq(const FuncPtr& v) {
        FuncPtr u(nullptr);
        if (auto ptr = std::dynamic_pointer_cast<ScalerFunction<AlgType>>(v)) {
            u = transform_freq_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<SumFunction<AlgType>>(v)) {
            u = transform_freq_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<ShiftFunction<AlgType>>(v)) {
            u = transform_freq_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<ProductFunction<AlgType>>(v)) {
            u = transform_freq_func(*ptr);
        }

        if (u) {
            time_func.compose_outer(u);
        } else {
            throw std::invalid_argument("Frequency compose using a function not trivially transformed into time space");
        } 
        freq_func.compose_outer(v);
    }

    // May throw std::invalid argument on bad function input
    void compose_time(const FuncPtr& u) {
        FuncPtr v(nullptr);
        if (auto ptr = std::dynamic_pointer_cast<ScalerFunction<AlgType>>(u)) {
            v = transform_time_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<SumFunction<AlgType>>(u)) {
            v = transform_time_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<ShiftFunction<AlgType>>(u)) {
            v = transform_time_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<ConjFunction<AlgType>>(u)) {
            v = transform_time_func(*ptr);
        } else if (auto ptr = std::dynamic_pointer_cast<ProductFunction<AlgType>>(u)) {
            v = transform_time_func(*ptr);
        }

        if (v) {
            freq_func.compose_outer(v);  
        } else {
            throw std::invalid_argument("Time composed using a function not trivially transformed into frequency space");
        }
        time_func.compose_outer(u);
    }
};
