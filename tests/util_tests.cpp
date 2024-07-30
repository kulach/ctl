#include <utest.h>
#include "test_utils.h"
#include <common.h>

UTEST(UtilTests, TestComplexSwap) {
    complex<double> a{1.0, 5.0};
    complex<double> b{2.0, 10.0};

    complexref<double> aref(a);
    complexref<double> bref(b);

    swap(aref, bref);

    EXPECT_TRUE(tutil::deq(a.re, 2.0));
    EXPECT_TRUE(tutil::deq(a.im, 10.0));
    EXPECT_TRUE(tutil::deq(b.re, 1.0));
    EXPECT_TRUE(tutil::deq(b.im, 5.0));

}

