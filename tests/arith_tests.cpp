#include <utest.h>
#include <arith.h>
#include "test_utils.h"

UTEST(ArithTests, TestDoubles) {
    using darith = arith<double>;
    alignas(darith::Alignment) double a[128];
    alignas(darith::Alignment) double b[128];
    for (int i = 0; i < 127; i++) {
        a[i] = 1.0 * i;
        b[i] = 1.0 * i;
    }

    arith<double>::_add_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 2.0 * i));

    arith<double>::_sub_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i));

    arith<double>::_mul_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i * i));

    arith<double>::_div_vec(a, a, b, 127);
    // Don't look at first index, it is a victim of dividing by 0
    for (int i = 1; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i));
}

UTEST(ArithTests, TestFloats) {
    float a[128];
    float b[128];
    for (int i = 0; i < 127; i++) {
        a[i] = 1.0 * i;
        b[i] = 1.0 * i;
    }

    arith<float>::_add_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 2.0 * i));

    arith<float>::_sub_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i));

    arith<float>::_mul_vec(a, a, b, 127);
    for (int i = 0; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i * i));

    arith<float>::_div_vec(a, a, b, 127);
    // Don't look at first index, it is a victim of dividing by 0
    for (int i = 1; i < 127; i++) EXPECT_TRUE(tutil::deq(a[i], 1.0 * i));
}
