#pragma once

#include <common.h>
#include <arith/basearith.h>
#include <complex.h>
#include <utility>

#if __AVX2__
#include <immintrin.h>
#endif


template<typename T> requires ComplexType<T>
struct carith {

    using BaseType = T::BaseType;
    using BaseRegType = BaseType;
    using OutputType = complexptr<BaseType>;
    using InputType = ccomplexptr<BaseType>;
    using RefType = ccomplexref<BaseType>;
    static constexpr size_t Alignment = sizeof(BaseType);
    static constexpr size_t OpCapacity = 1;
    
    struct _add_op_t {
        static inline void _exec(T& out, const RefType& a, const RefType& b) noexcept {
            out.re = a.re + b.re;
            out.im = a.im + b.im;
        }
    };

    struct _sub_op_t {
        static inline void _exec(T& out, const RefType& a, const RefType& b) noexcept {
            out.re = a.re - b.re;
            out.im = a.im - b.im;
        }
    };

    struct _mul_op_t {
        static inline void _exec(T& out, const RefType& a, const RefType& b) noexcept {
            T tmp;
            tmp.re = a.re * b.re - a.im * b.im;
            tmp.im = a.re * b.im + a.im * b.re;

            out.re = tmp.re;
            out.im = tmp.im;
        }
    };

    struct _faltmaddsub_op_t {
        // O_a = a + b * c
        // O_b = a - b * c
        static inline void _exec(T& outa, T& outb, const RefType& a, const RefType& b, const RefType& c) noexcept {
            T bc_prod;
            bc_prod.re = b.re * c.re - b.im * c.im;
            bc_prod.im = b.re * c.im + b.im * c.re;

            T a_tmp;
            a_tmp.re = a.re;
            a_tmp.im = a.im;

            outa.re = a.re + bc_prod.re;
            outa.im = a.im + bc_prod.im;
            outb.re = a.re - bc_prod.re;
            outb.im = a.im - bc_prod.im;
        }
    };

    struct _faltaddsubmultconj_t {
        // O_a = a + b
        // O_b = (a - b) * c*
        static inline void _exec(T& outa, T& outb, const RefType& a, const RefType& b, const RefType& c) noexcept {
            T ab_sum, b_diff;
            ab_sum.re = a.re + b.re;
            ab_sum.im = a.im + b.im;

            b_diff.re = a.re - b.re;
            b_diff.im = a.im - b.im;

            // Multiply by conjugation

            outb.re = b_diff.re * c.re + b_diff.im * c.im;
            outb.im = b_diff.im * c.re - b_diff.re * c.im;

            // Pollute outa
            outa.re = ab_sum.re;
            outa.im = ab_sum.im;
        }
    };

    template<typename Op, typename... Args>
    static inline void _vec_impl(Op, size_t n, OutputType& out, Args&&... args) noexcept {
        T _out;
        for (size_t i = 0; i < n; i++) {
            Op::_exec(_out, args[i]...); 
            out[i] = _out;
        }
    }

    template<typename Op, typename... Args>
    static inline void _vec_impl_2(Op, size_t n, OutputType& outa, OutputType& outb, Args&&... args) noexcept {
        T _outa, _outb;
        for (size_t i = 0; i < n; i++) {
            Op::_exec(_outa, _outb, args[i]...); 
            outa[i] = _outa;
            outb[i] = _outb;
        }
    }

    template <typename Op, typename... Args>
    static inline void _scalar_impl(Op, size_t n, OutputType& out, const InputType& a, const RefType& b) noexcept {
        T _out;
        for (size_t i = 0; i < n; i++) {
            Op::_exec(_out, a[i], b);
            out[i] = _out;
        }
    }

    static inline void _faltmaddsub_vec(OutputType outa, OutputType outb, InputType a, InputType b, InputType c, size_t n) noexcept {
        _vec_impl_2(_faltmaddsub_op_t{}, n, outa, outb, a, b, c);
    }

    static inline void _faltaddsubmultconj(OutputType outa, OutputType outb, InputType a, InputType b, InputType c, size_t n) noexcept {
        _vec_impl_2(_faltaddsubmultconj_t{}, n, outa, outb, a, b, c);
    }

    static inline void _add_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_add_op_t{}, n, out, a, b);
    }

    static inline void _sub_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_sub_op_t{}, n, out, a, b);
    }

    static inline void _mul_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_mul_op_t{}, n, out, a, b);
    }

    static inline void _mul_scalar(OutputType out, InputType a, RefType b, size_t n) noexcept {
        _scalar_impl(_mul_op_t{}, n, out, a, b);
    }
};

#if __AVX2__
template<>
struct carith<complex<double>> {


    using BaseType = double;
    using BaseRegType = __m256d;
    using OutputType = complexptr<double>;
    using InputType = ccomplexptr<double>;
    using RefType = ccomplexref<double>;
    static constexpr size_t Alignment = sizeof(__m256d);
    static constexpr size_t OpCapacity = (sizeof(__m256d) / sizeof(double));

    // The new reg type to use
    struct RegType {

        BaseRegType re;
        BaseRegType im;

        constexpr RegType(const InputType& ref, size_t offset) noexcept {
            re = _mm256_load_pd(ref.re + offset);
            im = _mm256_load_pd(ref.im + offset);
        }

        constexpr RegType() noexcept {
        }

        constexpr void load(InputType& ref, size_t offset) noexcept {
            re = _mm256_load_pd(ref.re + offset);
            im = _mm256_load_pd(ref.im + offset);
        }

        constexpr void store(OutputType& ref, size_t offset) noexcept {
            _mm256_store_pd(ref.re + offset, re);
            _mm256_store_pd(ref.im + offset, im);
        }
    };

    static_assert(OpCapacity == 4, "Bad assumption on SIMD register size");

    struct _add_op_t {
        static inline void _exec(RegType& out, const RegType& a, const RegType& b) noexcept {
            out.re = _mm256_add_pd(a.re, b.re);
            out.im = _mm256_add_pd(a.im, b.im);
        }
    };

    struct _sub_op_t {
        static inline void _exec(RegType& out, const RegType& a, const RegType& b) noexcept {
            out.re = _mm256_sub_pd(a.re, b.re);
            out.im = _mm256_sub_pd(a.im, b.im);
        }
    };

    struct _mul_op_t {
        static inline void _exec(RegType& out, const RegType& a, const RegType& b) noexcept {
            // Four mults and 2 adds necessary
            // aka 2 FMA and 2 mults
            // Cr = Ar * Br - Ai * Bi
            // Ci = Ar * Bi + Ai * Br
            out.re = _mm256_fmsub_pd(a.re, b.re, _mm256_mul_pd(a.im, b.im));
            out.im = _mm256_fmadd_pd(a.re, b.im, _mm256_mul_pd(a.im, b.re));
        }
    };

    struct _fma_op_t {
        static inline void _exec(RegType& out, const RegType& a, 
                const RegType& b, const RegType& c) noexcept {
            // Four mults and 4 adds necessary
            // Can compress into 4 FMA instructions!
            // D = A * B + C
            // Dr = Ar * Br - Ai * Bi + Cr
            // Di = Ar * Bi + Ai * Br + Ci
            out.re = _mm256_fmsub_pd(a.re, b.re, _mm256_fmadd_pd(a.im, b.im, c.re));
            out.im = _mm256_fmadd_pd(a.re, b.im, _mm256_fmadd_pd(a.im, b.re, c.im));
        }
    };

    struct _faltmaddsub_op_t {
        // O_a = a + b * c
        // O_s = a - b * c

        // O_a_r = a_r + (b_r * c_r - b_i * c_i)
        static inline void _exec(RegType& outa, RegType& outb, const RegType& a, const RegType& b, const RegType& c) noexcept {
            // Nothing yet
            RegType bc_prod;
            bc_prod.re = _mm256_fmsub_pd(b.re, c.re, _mm256_mul_pd(b.im, c.im));
            bc_prod.im = _mm256_fmadd_pd(b.re, c.im, _mm256_mul_pd(b.im, c.re));
            outa.re = _mm256_add_pd(a.re, bc_prod.re);
            outa.im = _mm256_add_pd(a.im, bc_prod.im);
            outb.re = _mm256_sub_pd(a.re, bc_prod.re);
            outb.im = _mm256_sub_pd(a.im, bc_prod.im);  
        }

    };
    struct _faltaddsubmultconj_t {
        // O_a = a + b
        // O_b = (a - b) * c* 
        // c* is the conjugation of c
        static inline void _exec(RegType& outa, RegType& outb, const RegType& a, const RegType& b, const RegType& c) noexcept {
            RegType ab_sum;
            RegType ab_diff;
            ab_sum.re = _mm256_add_pd(a.re, b.re);
            ab_sum.im = _mm256_add_pd(a.im, b.im);

            // ab_diff = (a-b)
            ab_diff.re = _mm256_sub_pd(a.re, b.re);
            ab_diff.im = _mm256_sub_pd(a.im, b.im);

            // Multiply by conjugation
            // a + jb * (c - jd) = ac + bd + j(bc - ad)

            outb.re = _mm256_fmadd_pd(ab_diff.re, c.re, _mm256_mul_pd(ab_diff.im, c.im));
            outb.im = _mm256_fmsub_pd(ab_diff.im, c.re, _mm256_mul_pd(ab_diff.re, c.im));
            

            // Pollute outa
            outa.re = ab_sum.re;
            outa.im = ab_sum.im;
        }
    };

    template<typename Op, typename... Args>
    static inline void _vec_impl_2(Op, size_t n, OutputType& outa, OutputType& outb, Args&&... args) noexcept {
        ASSERT(autil::_assert_align<Alignment>(outa.re, outa.im, outb.re, outb.im, args.re..., args.im...));
        RegType _outa, _outb;
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            Op::_exec(_outa, _outb, RegType(std::forward<Args>(args), offset)...);
            _outa.store(outa, offset);
            _outb.store(outb, offset);
        }
    }

    template<typename Op, typename... Args>
    static inline void _vec_impl(Op, size_t n, OutputType out, Args&&... args) noexcept {
        ASSERT(autil::_assert_align<Alignment>(out.re, out.im, args.re..., args.im...));
        RegType _out;
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            Op::_exec(_out, RegType(std::forward<Args>(args), offset)...);
            _mm256_store_pd(&out.re[offset], _out.re);
            _mm256_store_pd(&out.im[offset], _out.im);
        }
    }

    template <typename Op> 
    static inline void _scalar_impl(Op, size_t n, OutputType& out, InputType& a, RefType& b) noexcept {
        ASSERT(autil::_assert_align<Alignment>(out.re, out.im, a.re, a.im));
        RegType _b, _out;
        _b.re = _mm256_broadcast_sd(&b.re);
        _b.im = _mm256_broadcast_sd(&b.im);
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            Op::_exec(_out, RegType(a, offset), _b);
            _mm256_store_pd(&out.re[offset], _out.re);
            _mm256_store_pd(&out.im[offset], _out.im);
        }
    }

    static inline void _add_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_add_op_t{}, n, out, a, b);
    }

    static inline void _sub_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_sub_op_t{}, n, out, a, b);
    }

    static inline void _mul_vec(OutputType out, InputType a, InputType b, size_t n) noexcept {
        _vec_impl(_mul_op_t{}, n, out, a, b);
    }


    // O_a = a + b * c
    // O_s = a - b * c
    // Bespoke function with a scary looking name
    // It is used to compute the butterfly of the FFT without
    // having to go through multiple operations (linear memory lookup benefits)
    // faltaddsub stands for "Fused alternating Multiplied add/subtract"
    static inline void _faltmaddsub_vec(OutputType outa, OutputType outb, InputType a, InputType b, InputType c, size_t n) noexcept {
        _vec_impl_2(_faltmaddsub_op_t{}, n, outa, outb, a, b, c);
    }

    static inline void _faltaddsubmultconj(OutputType outa, OutputType outb, InputType a, InputType b, InputType c, size_t n) noexcept {
        _vec_impl_2(_faltaddsubmultconj_t{}, n, outa, outb, a, b, c);
        // _vec_impl_tup(_faltaddsubmultconj_t{}, n, outa, outb, a, b, c);
    }

    // FMA
    // D = A*B + C
    static inline void _fma_vec(OutputType out, InputType a, InputType b, InputType c, size_t n) noexcept { 
        _vec_impl(_fma_op_t{}, n, out, a, b, c);
    }
    
    static inline void _mul_scalar(OutputType out, InputType a, RefType b, size_t n) noexcept {
        _scalar_impl(_mul_op_t{}, n, out, a, b);
    }
};

#endif
