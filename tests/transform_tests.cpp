#include "common.h"
#include "function.h"
#include <utest.h>
#include <transform.h>
#include <memory>
#include "test_utils.h"

UTEST(TransformTests, TestFourierScale) {
    const size_t size = 16;
    FourierDual<double> fourier(size);
    CompositeFunction<complex<double>> comp;
    auto scale = std::make_shared<ScalerFunction<complex<double>>>(complex<double>{5.0, 0.0});
    comp.compose_outer(scale);
    fourier.set_time_func(comp);

    Vec<complex<double>> x{size};
    auto xview = tview::view(x);

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 2.0 * i;
        x.idata()[i] = -3.0 * i;
    }

    fourier.v(xview);

    EXPECT_TRUE(
        tutil::random_check<cplx128_t>(xview.data(), xview.size(), [](auto i) {
            return cplx128_t{10.0 * i, -15.0 * i};
        })
    );
}

UTEST(TransformTests, TestFourierAdd) {
    const size_t size = 32;
    FourierDual<double> fourier(size);
    CompositeFunction<cplx128_t> comp;
    
    Vec<cplx128_t> addend{size};
    Vec<cplx128_t> x{size};

    addend.zero();
    x.zero();
    auto xview = tview::view(x);
    
    addend.rdata()[0] = 1.0;

    size_t i = 0;
    for (auto x : xview) {
        x.re = 1.0 * i;
        x.im = 1.0 * i;
        i++;
    }

    auto adder = std::make_shared<SumFunction<cplx128_t>>(addend);
    comp.compose_outer(adder);

    fourier.set_time_func(comp);

    fourier.v(xview);

    EXPECT_TRUE(
        tutil::random_check<cplx128_t>(xview.data(), xview.size(), [](auto i) {
            return cplx128_t{1.0 + 1.0 * i, 1.0 * i};
        })
    );
}

UTEST(TransformTests, TestFourierShiftAndConj) {
    const size_t size = 32;
    FourierDual<double> fourier(size);
    CompositeFunction<cplx128_t> comp;

    auto shift = std::make_shared<ShiftFunction<cplx128_t>>(22);
    auto conj = std::make_shared<ConjFunction<cplx128_t>>();
    comp.compose_outer(shift);
    comp.compose_outer(conj);
    Vec<cplx128_t> input{size};
    Vec<cplx128_t> shifted{size};

    for (size_t i = 0; i < size; i++) {
        input.rdata()[i] = 1.0 * i;
        input.idata()[i] = 1.0 * i;
        shifted.rdata()[i] = 1.0 * i;
        shifted.idata()[i] = 1.0 * i;
    }

    fourier.set_time_func(comp);

    FFT<double> fft{size}; 

    // Shift the input
    comp(shifted);
    // FFT the modified input
    fft(shifted);


    // FFT the original input
    fft(input);
    // Use calculated transformation functions
    fourier.v(input);

    // Both shall be similar to one another
    auto iview = tview::view(input);
    auto sview = tview::view(shifted);

    EXPECT_TRUE(tutil::random_check<cplx128_t>(iview.data(), iview.size(), [&sview](auto i) {
            auto ref = sview[i];
            return cplx128_t{ref.re, ref.im};
        })
    );

}

UTEST(TransformTests, TestMult) {
    const size_t size = 16;
    FourierDual<double> fourier(size);
    CompositeFunction<complex<double>> comp;

    Vec<complex<double>> x{size};
    auto xview = tview::view(x);
    Vec<complex<double>> multiplicand{size};


    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 2.0 * i;
        x.idata()[i] = -3.0 * i;
        multiplicand.rdata()[i] = 2.0;
        multiplicand.idata()[i] = 0.0;
    }
    Vec<complex<double>> y{x};
    auto yview = tview::view(y);

    auto mult = std::make_shared<ProductFunction<complex<double>>>(multiplicand);
    comp.compose_outer(mult);
    fourier.set_time_func(comp);

    FFT<double> fft{size};
    
    xview = fourier.u(xview);
    xview = fft.fft(xview);

    fft.fft(yview);
    yview = fourier.v(yview);

    EXPECT_TRUE(
        tutil::random_eq(xview.data(), yview.data(), xview.size())
    );
}

UTEST(TransformTests, TestFreqScale) {
    const size_t size = 16;
    FourierDual<double> fourier(size);
    CompositeFunction<complex<double>> comp;
    auto scale = std::make_shared<ScalerFunction<complex<double>>>(complex<double>{5.0, 0.0});
    comp.compose_outer(scale);
    fourier.set_freq_func(comp);

    Vec<complex<double>> x{size};
    auto xview = tview::view(x);

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 2.0 * i;
        x.idata()[i] = -3.0 * i;
    }

    fourier.u(xview);

    EXPECT_TRUE(
        tutil::random_check<cplx128_t>(xview.data(), xview.size(), [](auto i) {
            return cplx128_t{10.0 * i, -15.0 * i};
        })
    );
}

UTEST(TransformTests, TestFreqShift) {
    const size_t size = 16;
    FourierDual<double> fourier(size);
    CompositeFunction<complex<double>> comp;

    Vec<complex<double>> x{size};
    auto xview = tview::view(x);


    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 2.0 * i;
        x.idata()[i] = -3.0 * i;
    }

    Vec<complex<double>> y{x};
    auto yview = tview::view(y);

    auto shift = std::make_shared<ShiftFunction<cplx128_t>>(7);
    comp.compose_outer(shift);
    fourier.set_freq_func(comp);

    FFT<double> fft{size};
    
    xview = fourier.u(xview);
    xview = fft.fft(xview);

    fft.fft(yview);
    yview = fourier.v(yview);

    EXPECT_TRUE(
        tutil::random_eq(xview.data(), yview.data(), xview.size())
    );
}

UTEST(TransformTests, TestFreqMult) {
    const size_t size = 16;
    FourierDual<double> fourier(size);
    CompositeFunction<complex<double>> comp;

    Vec<complex<double>> x{size};
    auto xview = tview::view(x);
    Vec<complex<double>> multiplicand{size};


    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 2.0 * i;
        x.idata()[i] = -3.0 * i;
        multiplicand.rdata()[i] = 2.0;
        multiplicand.idata()[i] = 0.0;
    }
    Vec<complex<double>> y{x};
    auto yview = tview::view(y);

    auto mult = std::make_shared<ProductFunction<complex<double>>>(multiplicand);
    comp.compose_outer(mult);
    fourier.set_freq_func(comp);

    FFT<double> fft{size};
    
    xview = fourier.u(xview);
    xview = fft.fft(xview);

    fft.fft(yview);
    yview = fourier.v(yview);

    EXPECT_TRUE(
        tutil::random_eq(xview.data(), yview.data(), xview.size())
    );
}
