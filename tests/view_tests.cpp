#include <utest.h>
#include "test_utils.h"
#include <tview.h>
#include <algorithm>

UTEST(SliceTests, TestComplex) {
    Vec<complex<double>> t{32};
    for (size_t i = 0; i < t.size(); i++) {
        t.rdata()[i] = 1.0 * i;
        t.idata()[i] = 1.0 * i;
    }

    //const ConstComplexView<double> view(t);
    ConstView<complex<double>> view(t);

    for (size_t i = 0; i < view.size() / 2; i++) {
        auto x = view[i];
        EXPECT_TRUE(tutil::deq(x.re, 1.0 * i));
        EXPECT_TRUE(tutil::deq(x.im, 1.0 * i));
    }

    MutView<complex<double>> mut_view(t);
    for (size_t i = 0; i < mut_view.size(); i++) {
        auto ref = mut_view[i];
        ref.re = 2.0;
        ref.im = 3.0;
    }

    for (size_t i = 0; i < view.size() / 2; i++) {
        auto x = view[i];
        EXPECT_TRUE(tutil::deq(x.re, 2.0));
        EXPECT_TRUE(tutil::deq(x.im, 3.0));
    }
    
    mut_view += mut_view;

    for (size_t i = 0; i < view.size() / 2; i++) {
        auto x = view[i];
        EXPECT_TRUE(tutil::deq(x.re, 4.0));
        EXPECT_TRUE(tutil::deq(x.im, 6.0));
    }

}

UTEST(SliceTests, TestScalar) {
    Vec<double> t{16};
    MutView<double> view(t);

    for (size_t i = 0; i < view.size(); i++) {
        view[i] = 1.0 * i;
    }

    view += view;
    
    for (size_t i = 0; i < view.size() / 2; i++) {
        EXPECT_TRUE(tutil::deq(view[i], 2.0 * i));
    }
}

UTEST(SliceTests, TestFloatingIterator) {
    Vec<double> t{16};
    MutView<double> view(t);
    size_t i = view.size();
    for (auto& a : view) {
        a = (--i) * 1.0;
    }

    std::ranges::rotate(view, view.begin() + 2);
    std::ranges::sort(view);

    i = 0;
    for (auto a : view) {
        EXPECT_TRUE(tutil::deq(a, 1.0 * i++));
    }
}

UTEST(SliceTests, TestMergeAlgorithm) {
    Vec<double> t{32};
    MutView<double> view(t);
    size_t i = 0;
    for (auto& a : view) {
        a = 1.0 * ((9 * i++) % 16); // 9 does not divide 16, we should be able to fill in each element uniquely
                                  // 0 -> 9 -> 2 -> 11 -> 3 -> ...
    }

    std::ranges::sort(view.begin(), view.begin() + 8);
    std::ranges::sort(view.begin() + 8, view.begin() + 16);
    std::ranges::merge(view.begin(), view.begin() + 8, view.begin() + 8, view.begin() + 16, view.begin() + 16);
    
    i = 0;
    for (const auto& a : std::ranges::subrange(view.begin() + 16, view.end())) {
        EXPECT_TRUE(tutil::deq(a, 1.0 * i++));
    }

}

UTEST(SliceTests, TestComplexIterator) {
    Vec<complex<double>> t{16};
    MutView<complex<double>> view(t);

    size_t i = 0;
    for (auto a : view) {
        a.re = i * 2.0;
        a.im = i * 3.0;
        i++;
    }

    // There is no ordering of complex numbers
    // At least none that I know of 
    std::ranges::rotate(view, view.begin() + 8);
    std::ranges::copy(std::ranges::subrange(view.begin(), view.begin() + 8), view.begin() + 8);

    i = 0;
    for (auto a : view) {
        // EXPECT_TRUE(tutil::deq(a, 1.0 * i++));
        auto shifted = i % 8 + 8;
        EXPECT_TRUE(tutil::eq(a, {2.0 * shifted, 3.0 * shifted}));
        i++;
    }
}
