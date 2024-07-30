#include <utest.h>
#include "test_utils.h"
#include <convolve.h>

UTEST(ConvolutionTests, SimpleConvolution) {
    size_t SIZE = 32;
    Vec<complex<double>> x{SIZE};
    Vec<complex<double>> h{SIZE};
    auto xview = tview::view(x);

    for (size_t i = 0; i < SIZE; i++) {
        x.rdata()[i] = i * 1.0;
        x.idata()[i] = i * 1.0;
        h.rdata()[i] = 0.0;
        h.idata()[i] = 0.0;
    }

    h.rdata()[2] = 2.0;

    ConvolutionFunction<complex<double>> conv(std::move(h));
    conv(x);

    EXPECT_TRUE(tutil::random_check<cplx128_t>(xview.data(), xview.size(), [&SIZE](size_t i) {
        return cplx128_t{((i-2) % SIZE) * 2.0, ((i-2) % SIZE) * 2.0};
    }));

}
