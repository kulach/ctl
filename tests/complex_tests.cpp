#include <utest.h>
#include <vec.h>
#include "test_utils.h"
#include <tview.h>

UTEST(ComplexTests, TestComplexVec) {
    Vec<complex<double>> data({128});
    EXPECT_EQ(data.size(), 128u);
    EXPECT_EQ(data.rdata() + 128u, data.idata());

    double* re = data.rdata();
    double* im = data.idata();

    for (uint32_t i = 0; i < 128; i++) {
        re[i] = i * 2.0;
        im[i] = i * 3.0;
    }

    auto cpy = data;
    for (uint32_t i = 0; i < 32; i++) {
        EXPECT_TRUE(tutil::deq(cpy.data()[i], data.data()[i]));
    }

    auto mov = std::move(data);
    EXPECT_EQ(data.size(), 0u);
}

UTEST(ComplexTests, TestComplexArith) {
    Vec<complex<double>> data({50});
    double* re = data.rdata();
    double* im = data.idata();

    auto view = tview::view(data);

    EXPECT_LE(data.size(), 52u);
    EXPECT_EQ(data.size() % carith<complex<double>>::OpCapacity, 0u);

    for (uint32_t i = 0; i < 50; i++) {
        re[i] = 0.0;
        im[i] = i * 2.0;
    }

    view *= tview::view(data);

    for (uint32_t i = 0; i < data.size() / 4; i++) {
        EXPECT_TRUE(tutil::deq(re[i], i * i * (-4.0)));
        EXPECT_TRUE(tutil::deq(im[i], 0.0));
    }
}

UTEST(ComplexTests, TestAlternatingMultAdd) {
    Vec<complex<double>> a({50});
    Vec<complex<double>> b({50});
    Vec<complex<double>> c({50});

    for (int32_t i = 0; i < 50; i++) {
        a.rdata()[i] = 1.0 * i;
        a.idata()[i] = 0.0;

        b.rdata()[i] = 1.0 * i;
        b.idata()[i] = 0.0;

        c.rdata()[i] = 0.0;
        c.idata()[i] = 1.0 * i;
    }

    auto viewa = tview::view(a);
    auto viewb = tview::view(b);
    auto viewc = tview::view(c);

    // VecSlice<complex<double>>::altAddSubProd(viewa, viewb, a, b, c);
    altAddSubProd(viewa, viewb, viewc);

    EXPECT_TRUE(tutil::random_check<complex<double>>(viewa.data(), 50, [](size_t i) {
        return complex<double>{1.0 * i, 1.0 * i * i};
    }));

    EXPECT_TRUE(tutil::random_check<complex<double>>(viewb.data(), 50, [](size_t i) {
        return complex<double>{1.0 * i, -1.0 * i * i};
    }));
}
