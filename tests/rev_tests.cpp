#include "tests/test_utils.h"
#include <utest.h>
#include <shuffler.h>

UTEST(BitReversalTests, TestBase) {
    Vec<int> t{1};
    ShuffleFunction<int> shuffle(t.size());
    EXPECT_EQ(t.size(), 1u);
    int* arr = t.data();
    *arr = 5.0;

    // t = shuffle(std::move(t));
    //t = shuffle.exec(std::move(t));
    auto s = shuffle(t);
    EXPECT_TRUE(tutil::deq(*s.data(), 5.0));
}

UTEST(BitReversalTests, TestTiny) {
    Vec<int> t{8};
    ShuffleFunction<int> shuffle(t.size());
    int* arr = t.data();
    for (size_t i = 0; i < t.size(); i++) {
        arr[i] = i;
    }

    auto s = shuffle(t);
    arr = s.data();
    
    EXPECT_EQ(arr[0b000], 0b000);
    EXPECT_EQ(arr[0b001], 0b100);
    EXPECT_EQ(arr[0b010], 0b010);
    EXPECT_EQ(arr[0b011], 0b110);
    EXPECT_EQ(arr[0b100], 0b001);
    EXPECT_EQ(arr[0b101], 0b101);
    EXPECT_EQ(arr[0b110], 0b011);
    EXPECT_EQ(arr[0b111], 0b111);
}

UTEST(BitReversalTests, TestSmall) {
    Vec<int> t{64};
    ShuffleFunction<int> shuffle(t.size());
    int* arr = t.data();
    for (size_t i = 0; i < t.size(); i++) {
        arr[i] = i;
    }

    auto s = shuffle(t);
    arr = s.data();
    
    EXPECT_EQ(arr[0b100000], 0b000001);
    EXPECT_EQ(arr[0b100001], 0b100001);
    EXPECT_EQ(arr[0b101101], 0b101101);
    EXPECT_EQ(arr[0b100011], 0b110001);
    EXPECT_EQ(arr[0b000000], 0b000000);
    EXPECT_EQ(arr[0b111111], 0b111111);
    EXPECT_EQ(arr[0b101010], 0b010101);
}

UTEST(BitReversalTests, TestLarge) {
    size_t nbits = 12;
    size_t size = util::pow2(12); // 4096 in this case
    Vec<int> t{size};
    ShuffleFunction<int> shuffle(t.size());
    int* arr = t.data();
    for (size_t i = 0; i < t.size(); i++) {
        arr[i] = i;
    }

    auto s = shuffle(t);
    arr = s.data();

    EXPECT_TRUE(tutil::random_check<int>(arr, size, [nbits](size_t i) {
            return shuffle::rev_int(i, nbits);
        }));
    
    // EXPECT_EQ(arr[0b1000000000], 0b0000000001);
    // EXPECT_EQ(arr[0b1111100001], 0b1000011111);
    // EXPECT_EQ(arr[0b1001101101], 0b1011011001);
    // EXPECT_EQ(arr[0b1000110000], 0b0000110001);
    // EXPECT_EQ(arr[0b0000000000], 0b0000000000);
    // EXPECT_EQ(arr[0b1111111111], 0b1111111111);
    // EXPECT_EQ(arr[0b1010101010], 0b0101010101);
}

UTEST(BitReversalTests, TestComplex) {
    Vec<complex<double>> t{16};

    for (size_t i = 0; i < t.size(); i++) {
        t.rdata()[i] = 1.0 * i;
        t.idata()[i] = 2.0 * i;
    }

    ShuffleFunction<complex<double>> shuffle(t.size());

    auto s = shuffle(t);
    auto data = s.data();

    EXPECT_TRUE(tutil::ceq(data[0b0000], complex<double>{0.0, 0.0}));

    EXPECT_TRUE(tutil::ceq(data[0b0100], complex<double>{2.0, 4.0}));

    EXPECT_TRUE(tutil::ceq(data[0b0010], complex<double>{4.0, 8.0}));

    EXPECT_TRUE(tutil::ceq(data[0b1111], complex<double>{15.0, 30.0}));
}

