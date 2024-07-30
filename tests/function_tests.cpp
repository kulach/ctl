#include <utest.h>
#include <function.h>
#include "common.h"
#include "test_utils.h"

UTEST(FunctionTests, TestIdentity) {
    Vec<double> t{256};
    for (size_t i = 0; i < t.size(); i++) {
        double* arr = t.data();
        arr[i] = 1.0 * (i+1);
    }

    IdentityFunction<double> fid;
 
    auto res = fid(t);

    EXPECT_EQ(res.size(), t.size());
    EXPECT_TRUE(tutil::random_check<double>(t.data(), t.size(), 
                [](size_t i) {return 1.0 * (i+1);}));
}

UTEST(FunctionTests, TestScale) {
    Vec<double> t{256};
    for (size_t i = 0; i < t.size(); i++) {
        double* arr = t.data();
        arr[i] = 1.0 * (i+1);
    }

    ScalerFunction<double> scale(5.0);
 
    auto res = scale(t);
    EXPECT_EQ(res.size(), t.size());
    EXPECT_TRUE(tutil::random_check<double>(t.data(), t.size(), 
                [](size_t i) {return 5.0 * (i+1);}));
}

UTEST(FunctionTests, TestComplexScale) {
    Vec<cplx128_t> t{32};
    // EXPECT_TRUE(true);
    double* re = t.rdata();
    double* im = t.idata();

    for (size_t i = 0; i < t.size(); i++) {
        re[i] = 1.0 * i;
        im[i] = 2.0 * i;
    }
    
    ScalerFunction<cplx128_t> scaler({1.0, 2.0});

    auto res = scaler(t);

    EXPECT_TRUE(
        tutil::random_check<complex<double>>(res.data(), res.size(), 
            [](size_t i) {return complex<double>{1.0 * i - 4.0 * i, 4.0 * i};}
    ));
}

UTEST(FunctionTests, TestAdd) {
    Vec<double> t{256};
    for (size_t i = 0; i < t.size(); i++) {
        double* arr = t.data();
        arr[i] = 1.0 * (i+1);
    }

    AddFunction<double> add(5.0);
 
    auto res = add(t);
    EXPECT_EQ(res.size(), t.size());

    EXPECT_TRUE(tutil::random_check<double>(t.data(), t.size(), 
                [](size_t i) {return 1.0*(i+1) + 5.0;}));
}

UTEST(FunctionTests, TestLookup) {
    Vec<double> f{256};
    Vec<double> x{128};
    for (size_t i = 0; i < f.size(); i++) {
        double* arr = f.data();
        arr[i] = 5.0 * (i+1);
    }

    for (size_t i = 0; i < x.size(); i++) {
        double* arr = x.data();
        arr[i] = 1.0 * i;
    }
    
    MapFunction map(f);

    auto res = map(x);

    EXPECT_EQ(res.size(), 128u);

    EXPECT_TRUE(tutil::random_check<double>(res.data(), res.size(), 
                [&f](size_t i) {return f.data()[i];}));
}

UTEST(FunctionTests, TestComposite) {
    Vec<double> x{128};
    double* arr = x.data();
    for (size_t i = 0; i < x.size(); i++) {
        arr[i] = 1.0 * i;
    }

    CompositeFunction<double> comp;
    auto mult = std::make_shared<ScalerFunction<double>>(5.0);
    auto add = std::make_shared<AddFunction<double>>(10.0);
    // comp.compose_outer(std::reinterpret_cast<std::shared_ptr<BaseFunction<double>>>(mult));
    comp.compose_outer(mult);
    comp.compose_outer(add);

    auto slice = comp(x);

    arr = slice.data();
    for (size_t i = 0; i < x.size(); i++) {
        EXPECT_TRUE(tutil::deq(arr[i], 5.0 * i + 10.0));
    }
}

UTEST(FunctionTests, TestShift) {
    Vec<complex<double>> t{32};
    auto view = tview::view(t);
    size_t i = 0;
    for (auto c : view) {
        c.re = 1.0 * i;
        c.im = 2.0 * i;
        i++;
    }

    ShiftFunction<complex<double>> shift(5);

    shift(view);

    i = 0;
    for (auto c : view) {
        auto shifted = (i - 5) % 32;
        EXPECT_TRUE(tutil::eq(c, {1.0 * shifted, 2.0 * shifted}));
        i++;
    }

}

UTEST(FunctionTests, TestSum) {
    Vec<double> addend{32};
    Vec<double> t{32};
    auto aview = tview::view(addend);
    auto teview = tview::view(t);

    for (size_t i = 0; i < aview.size(); i++) {
        aview[i] = 1.0 * i;
        teview[i] = 4.0 * i;
    }

    SumFunction<double> summer(std::move(addend));

    teview = summer(teview);

    size_t i = 0;
    for (auto& x : teview) {
        EXPECT_TRUE(tutil::deq(x, 5.0 * i++));
    }
}

UTEST(FunctionTests, TestConj) {
    Vec<cplx128_t> t{32};
    auto tview = tview::view(t);
    double* re = t.rdata();
    double* im = t.idata();

    for (size_t i = 0; i < t.size(); i++) {
        re[i] = 1.0 * i;
        im[i] = 2.0 * i;
    }
    
    ConjFunction<cplx128_t> conj;

    conj(tview::view(t));

    EXPECT_TRUE(
        tutil::random_check<complex<double>>(tview.data(), tview.size(), 
            [](size_t i) {return complex<double>{1.0 * i, -2.0 * i};}
    ));
}

UTEST(FunctionTests, TestReverse) {
    Vec<cplx128_t> t{32};
    auto tview = tview::view(t);
    double* re = t.rdata();
    double* im = t.idata();

    for (size_t i = 0; i < t.size(); i++) {
        re[i] = 1.0 * i;
        im[i] = 2.0 * i;
    }
    
    CircularReverse<cplx128_t> rev;

    rev(tview::view(t));

    EXPECT_TRUE(
        tutil::random_check<complex<double>>(tview.data(), tview.size(), 
            [&tview](size_t i) {
            size_t new_index = (-i) % tview.size();
            return complex<double>{1.0 * new_index, 2.0 * new_index};}
    ));
}
