#pragma once

#include "common.h"
#include <function.h>

namespace revutil {
    static constexpr uint8_t revtable[] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
    };
}

namespace shuffle {
    // TODO -- Try to make this constexpr
    inline size_t rev_int(size_t num, size_t bits) noexcept {
        constexpr auto nbytes = sizeof(size_t);
        static_assert(nbytes % 2 == 0);
        uint8_t* word = reinterpret_cast<uint8_t*>(&num);
        for (uint32_t i = 0; i < nbytes / 2; i++) {
            uint8_t* a = &word[i];
            uint8_t* b = &word[nbytes - 1 - i];
            *a = (*a) ^ (*b); // a = a ^ b
            *b = (*a) ^ (*b); // b = a ^ b ^ b = a
            *a = (*a) ^ (*b); // a = a ^ b ^ b ^ a ^ b = b
            *a = revutil::revtable[*a];
            *b = revutil::revtable[*b];
        }
        num = num >> (8 * nbytes - bits);
        return num;
    }

    constexpr size_t num_bits(size_t n) noexcept {
        // Number of bits necessary to represent numbers in [0,n)
        // 7 -> 3
        // 8 -> 3
        // 9 -> 4 
        // 16 -> 4
        // 17 -> 5
        size_t max = n - 1;
        size_t tot_bits = 8 * sizeof(n); // 8 bits per byte
        size_t bits = tot_bits - std::countl_zero(max); // Count leading zeros of maximum to find logarithm
        return bits;
    }

};


// ScalarType is only enforced because the operator() 
// indexing would break upon usage
template <typename T> requires ArithType<T>
class ShuffleFunction : public BaseFunction<T> {
    using RevPair = std::pair<size_t, size_t>;

    // Size of a and c addressors -- used for fast shuffler
    // Let it be known that 5 is not a randomly chosen number
    // This number will allow the tmp buffer to be of size 2^(2*Q) = 1024
    // 1024 * 2 * sizeof(double) on x86 will be 16kB, which is half the size of my L1 cache 
    // on my personal desktop, perfect for a wanted swappable temporary buffer
    const size_t Q = 5;

    std::vector<RevPair> pairs;
    size_t _size;

    private:
    void generate_pairs() {
        size_t bits = shuffle::num_bits(_size);
        for (size_t i = 0; i < _size; i++) {
            size_t rev = shuffle::rev_int(i, bits);
            if (i < rev) {
                // if i == rev, we do not need to switch them anyway so we will
                // leave them out of this one
                pairs.emplace_back(i, rev);
            }
        }
    }

    void shuffle_impl(MutView<T>& input) const {
        using std::swap;
        for (auto& [a, b] : pairs) {
            typename MutView<T>::RefType aref = input[a];
            typename MutView<T>::RefType bref = input[b];
            swap(aref, bref);
            // swap(std::forward<decltype(aref)>(aref), std::forward<decltype(bref)>(bref));
        }
    }

    void shuffle_impl_trivial(MutView<T>& input) const {
        using std::swap;
        size_t nbits = shuffle::num_bits(_size);
        for (size_t i = 0; i < _size; i++) {
            auto ip = shuffle::rev_int(i, nbits);
            if (i < ip) {
                swap(input[i], input[ip]);
            }
        }
    }

    // TODO -- Make this look pretty
    static constexpr int64_t concat(size_t Q, size_t B_SIZE, int64_t a, int64_t b, int64_t c) noexcept {
        return (a << (Q+B_SIZE)) | (b << (Q)) | c;
    }

    static constexpr int64_t concat_2(size_t Q, int64_t a, int64_t c) noexcept {
        return a << Q | c;
    }

    // Algorithm derived from:
    //
    // Towards an Optimal Bit-Reversal Permutation Program
    // Larry Carter and Kang Su Gatlin
    // UC San Diego Department of Computer Science and Engineering
    // https://ieeexplore.ieee.org/document/743505
    void shuffle_impl_cobra(MutView<T>& input) const {
        // Pseudo code as said in the paper itself is as follows:
        //
        
        //for b = 0 to 2ˆ(lgN-2q)-1
        //  b’ = r(b)
        //  for a = 0 to 2ˆq-1
        //      a’ = r(a)
        //      for c = 0 to 2ˆq-1 [1]
        //      T[a’c] = A[abc]
        //
        //  for c = 0 to 2ˆq-1
        //      c’ = r(a) [sic]
        //      for a’ = 0 to 2ˆq-1 [2]
        //          B[c’b’a’] = T[a'c]

        size_t nbits = shuffle::num_bits(_size);
        Vec<T> tmp{util::pow2(2*Q)};
        auto tview = tview::view(tmp);
        // Vec<T> output_copy{input.size()};
        // auto oview = tview::view(output_copy);
        
        using std::swap;

        for (uint64_t b = 0; b < util::pow2(nbits - 2*Q); b++) {
            // rev_int() may not be optimized -- be aware of its use
            auto bp = shuffle::rev_int(b, nbits-2*Q);
            if (b > bp) continue; // We have already swapped this 
                                  
            for (uint64_t a = 0; a < util::pow2(Q); a++) {
                auto ap = shuffle::rev_int(a, Q);
                for (uint64_t c = 0; c < util::pow2(Q); c++) {
                    // TODO -- make some sick data structure that concats everything for you
                    auto t_index = concat_2(Q, ap, c);
                    auto a_index = concat(Q, nbits-2*Q, a, b, c);
                    // Here, t is currently in (a' c) ordering
                    // T[a'c] = a[abc]
                    tview[t_index] = input[a_index];
                }
            }

            for (uint64_t c = 0; c < util::pow2(Q); c++) {
                auto cp = shuffle::rev_int(c, Q);
                for (uint64_t ap = 0; ap < util::pow2(Q); ap++) {
                    auto t_index = concat_2(Q, ap, c);
                    auto b_index = concat(Q, nbits-2*Q, cp, bp, ap);
                    // Oops! I made a completely new container since I did not feel like implementing swapping
                    // logic by myself! I am merely doing this as a proof of concept!
                    // oview[b_index] = tview[t_index];
                    // if (b == bp) {
                    //    input[b_index] = tview[t_index];
                    //} else {
                        // T here is in (a' c) ordering, and it swaps with input[*bp*]
                        // input[c'b'a'] <-> T[a'c]
                        // Which implies that input[*b'*] is in (c'b'a') ordering
                        // while T now is in (a c') ordering
                        swap(input[b_index], tview[t_index]);
                        // Thus, we must redo the operation with the original b

                    //}
                }
            }
            // Move the swapped data back into the original line by using the same pattern
            if (b != bp) {
                for (uint64_t a = 0; a < util::pow2(Q); a++) {
                    auto ap = shuffle::rev_int(a, Q);
                    for (uint64_t c = 0; c < util::pow2(Q); c++) {
                        // TODO -- make some sick data structure that concats everything for you
                        auto t_index = concat_2(Q, ap, c);
                        auto a_index = concat(Q, nbits-2*Q, a, b, c);
                        // Here, t is currently in (a' c) ordering
                        // T[a'c] = a[abc]
                        input[a_index] = tview[t_index];
                    }
                }

            }
        }

        // TODO -- Remove dependent copy
        // for (size_t i = 0; i < input.size(); i++) {
        //     input[i] = oview[i];
        // }
    }

    public:
    ShuffleFunction(size_t n) : _size(n) {
        ASSERT(util::is_pow2(n));
        ASSERT(n >= 1);
        // pairs.reserve(n / 2);
        // generate_pairs();
    }

    MutView<T> operator()(MutView<T> input) const override {
        // shuffle_impl(input);
        if (_size <= util::pow2(2*Q)) {
            // shuffle_impl(input);
            shuffle_impl_trivial(input);
        } else {
            shuffle_impl_cobra(input);
        }
        
        return input;
    }

    inline size_t size() const noexcept {
        return _size;
    }

    size_t input_size() const override {
        return 1;
    }
};
