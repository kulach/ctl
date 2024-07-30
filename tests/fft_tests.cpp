#include <numbers>
#include <random>
#include <utest.h>
#include <twiddle.h>
#include "test_utils.h"
#include <fft.h>

UTEST(FFTTests, TestSize2) {
    FFT<double> fft(2);
    Vec<complex<double>> x{2};

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 1.0 * (i+1);
        x.idata()[i] = 2.0 * (i+1);
    }

    auto s = fft(x);

    EXPECT_TRUE(tutil::eq(s[0], complex<double>(3.0, 6.0)));
    EXPECT_TRUE(tutil::eq(s[1], complex<double>(-1.0, -2.0))); 
}

UTEST(FFTTests, TestSize4) {
    FFT<double> fft(4);
    Vec<complex<double>> x{4};

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 1.0 * (i+1);
        x.idata()[i] = -1.0 * (i+1);
    }

    auto s = fft(x);

    EXPECT_TRUE(tutil::eq(s[0], complex<double>(10.0, -10.0)));
    EXPECT_TRUE(tutil::eq(s[1], complex<double>(0.0, 4.0))); 
    EXPECT_TRUE(tutil::eq(s[2], complex<double>(-2.0, 2.0))); 
    EXPECT_TRUE(tutil::eq(s[3], complex<double>(-4.0, 0.0))); 
}

UTEST(FFTTests, TestOtherSize4) {
    FFT<double> fft(4);
    Vec<complex<double>> x{4};

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 1.0 * i;
        x.idata()[i] = 1.0 * i;
    }

    auto s = fft(x);

    EXPECT_TRUE(tutil::eq(s[0], complex<double>(6.0, 6.0)));
    EXPECT_TRUE(tutil::eq(s[1], complex<double>(-4.0, 0.0))); 
    EXPECT_TRUE(tutil::eq(s[2], complex<double>(-2.0, -2.0))); 
    EXPECT_TRUE(tutil::eq(s[3], complex<double>(0.0, -4.0))); 
}

UTEST(FFTTests, TestEvenSize8) {
    FFT<double> fft(8);
    Vec<complex<double>> x{8};

    for (size_t i = 0; i < x.size(); i++) {
        if (i % 2 == 0) {
            x.rdata()[i] = 1.0 * i / 2;
            x.idata()[i] = 1.0 * i / 2;
        } else {
            x.rdata()[i] = 0.0;
            x.idata()[i] = 0.0;
        }
    }

    auto s = fft(x);

    EXPECT_TRUE(tutil::eq(s[0], complex<double>(6.0, 6.0)));
    EXPECT_TRUE(tutil::eq(s[1], complex<double>(-4.0, 0.0))); 
    EXPECT_TRUE(tutil::eq(s[2], complex<double>(-2.0, -2.0))); 
    EXPECT_TRUE(tutil::eq(s[3], complex<double>(0.0, -4.0))); 

    EXPECT_TRUE(tutil::eq(s[4], complex<double>(6.0, 6.0)));
    EXPECT_TRUE(tutil::eq(s[5], complex<double>(-4.0, 0.0))); 
    EXPECT_TRUE(tutil::eq(s[6], complex<double>(-2.0, -2.0))); 
    EXPECT_TRUE(tutil::eq(s[7], complex<double>(0.0, -4.0)));  
}

UTEST(FFTTests, TestPulseSize32) {
    const size_t SIZE = 32;
    std::random_device r;
    std::default_random_engine engine(r());
    std::uniform_int_distribution<size_t> gen(0, SIZE-1);
    std::uniform_real_distribution ampgen(-100.0, 100.0);

    FFT<double> fft(SIZE);
    Vec<complex<double>> x{SIZE};

    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 0.0;
        x.idata()[i] = 0.0;
    }
    double ramp = ampgen(engine);
    double iamp = ampgen(engine);
    size_t r_pulse = gen(engine);
    size_t i_pulse = gen(engine);

    x.rdata()[r_pulse] = ramp;
    x.idata()[i_pulse] = iamp;

    auto s = fft(x);

    EXPECT_TRUE(tutil::random_check<complex<double>>(s.data(), s.size(), [r_pulse, i_pulse, ramp, iamp](size_t i) {
        double rangle = -2.0 * std::numbers::pi * (1.0 * i * r_pulse) / (1.0 * SIZE);
        double iangle = -2.0 * std::numbers::pi * (1.0 * i * i_pulse) / (1.0 * SIZE);
        return complex<double>{ramp * cos(rangle) - iamp * sin(iangle), ramp * sin(rangle) + iamp * cos(iangle)};
    }));
}

UTEST(FFTTests, TestIFFT) {
    const size_t SIZE = 32;
    FFT<double> fft(SIZE);
    Vec<complex<double>> x{SIZE};
    
    for (size_t i = 0; i < x.size(); i++) {
        x.rdata()[i] = 1.0 * i;
        x.idata()[i] = 2.0 * i;
    }
    x.rdata()[1] = 1.0;
    
    auto s = fft.fft(x);
    s = fft.ifft(s);

    EXPECT_TRUE(tutil::random_check<cplx128_t>(s.data(), s.size(), [](size_t i) {
        return cplx128_t{1.0 * i, 2.0 * i};
    }));
    
}
