#include <utest.h>
#include <vec.h>
#include <tview.h>

UTEST(VecTests, TestConstructor) {
    Vec<float> empty{};
    EXPECT_EQ(empty.ndim(), 0u);
    EXPECT_EQ(empty.size(), 0u);

    Vec<double> one_dim{16};
    EXPECT_EQ(one_dim.ndim(), 1u);
    EXPECT_EQ(one_dim.size(), 16u);

    Vec<uint32_t> two_dim{16, 8};
    EXPECT_EQ(two_dim.ndim(), 2u);
    EXPECT_EQ(two_dim.size(), 128u);

    Vec<uint32_t> three_dim{16, 8, 4};
    EXPECT_EQ(three_dim.ndim(), 3u);
    EXPECT_EQ(three_dim.size(), 512u);
}

UTEST(VecTests, TestCopy) {
    Vec<int> orig{8, 8};
    int* arr = orig.data();
    for (size_t i = 0; i < orig.size(); i++) {
        arr[i] = i;
    }
    Vec<int> copy(orig);
    arr = copy.data();
    for (size_t i = 0; i < orig.size(); i++) {
        EXPECT_EQ(arr[i], (int) i);
    }


    Vec<int> copy_eq{8};

    copy_eq = copy;

    arr = copy_eq.data();
    for (size_t i = 0; i < orig.size(); i++) {
        EXPECT_EQ(arr[i], (int) i);
    }
    EXPECT_NE(orig.data(), copy.data());
    EXPECT_NE(orig.data(), copy_eq.data());
    EXPECT_NE(copy.data(), copy_eq.data());

    EXPECT_EQ(copy.size(), orig.size());
    EXPECT_EQ(copy_eq.size(), orig.size());
}

UTEST(VecTests, TestMove) {
    Vec<int> orig{8, 8};
    int* arr = orig.data();
    for (size_t i = 0; i < orig.size(); i++) {
        arr[i] = i;
    }
    Vec<int> moved(std::move(orig));

    EXPECT_EQ(orig.data(), nullptr);
    EXPECT_EQ(orig.size(), 0u);
    EXPECT_EQ(orig.ndim(), 0u);

    EXPECT_NE(moved.data(), nullptr);
    EXPECT_EQ(moved.size(), 64u);
    EXPECT_EQ(moved.ndim(), 2u);

    Vec<int> moved_eq;

    moved_eq = std::move(moved);

    EXPECT_EQ(moved.data(), nullptr);
    EXPECT_EQ(moved.size(), 0u);
    EXPECT_EQ(moved.ndim(), 0u);

    EXPECT_NE(moved_eq.data(), nullptr);
    EXPECT_EQ(moved_eq.size(), 64u);
    EXPECT_EQ(moved_eq.ndim(), 2u);
}

UTEST(VecTests, TestSliceArith) {
    Vec<int> t{8, 8};
    int* arr = t.data();
    for (size_t i = 0; i < t.size(); i++) {
        arr[i] = i;
    }

    auto slice = tview::view(t);
    EXPECT_EQ(slice.size(), t.size());
    slice += (slice *= 2);

    arr = slice.data();
    for (int i = 0; i < static_cast<int>(slice.size()); i++) {
        EXPECT_EQ(arr[i], 4 * i);
    }
}
