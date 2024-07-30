#include <utest.h>
#include <twiddle.h>
#include "test_utils.h"
#include <cmath>

UTEST(TwiddleTests, TestLayers) {
    const TwiddleStore<double> twiddles{64};
    constexpr auto PI = decltype(twiddles)::PI;

    auto layer = twiddles.get_layer(64);

    EXPECT_TRUE(tutil::random_check<complex<double>>(layer.data(), layer.size(), [PI](auto i) {
        double angle = -2.0 * PI * (1.0 * i) / 64.0;
        return complex<double>{cos(angle), sin(angle)};
    }));

    layer = twiddles.get_layer(16);

    EXPECT_TRUE(tutil::random_check<complex<double>>(layer.data(), layer.size(), [PI](auto i) {
        double angle = -2.0 * PI * (1.0 * i) / 16.0;
        return complex<double>{cos(angle), sin(angle)};
    }));

    layer = twiddles.get_layer(4);
    EXPECT_TRUE(tutil::eq(layer[0], complex<double>{1.0, 0.0}));
    EXPECT_TRUE(tutil::eq(layer[1], complex<double>{0.0, -1.0}));

    layer = twiddles.get_layer(2);
    EXPECT_TRUE(tutil::eq(layer[0], complex<double>{1.0, 0.0}));
}
