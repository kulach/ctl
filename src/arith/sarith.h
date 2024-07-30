#pragma once

#include <common.h>
#include <arith/basearith.h>

#if __AVX2__
#include <immintrin.h>
#else
#pragma message("Compiling without AVX2 instructions")
#endif

template<typename T> requires ScalarType<T>
struct arith {

    static constexpr size_t Alignment = alignof(T);

    struct _add_op_t {
        static inline void _exec(T& c, const T& a, const T& b) noexcept {
            c = a + b;
        }
    };
    struct _sub_op_t {
        static inline void _exec(T& c, const T& a, const T& b) noexcept {
            c = a - b;
        }
    };
    struct _mul_op_t {
        static inline void _exec(T& c, const T& a, const T& b) noexcept {
            c = a * b;
        }
    };
    struct _div_op_t {
        static inline void _exec(T& c, const T& a, const T& b) noexcept {
            c = a / b;
        }
    };

    template <typename Op> requires VecOp<Op, T>
    static inline void _vec_impl(T* c, const T* a, const T* b, size_t n, Op) noexcept {
        for (size_t i = 0; i < n; i++) {
            Op::_exec(c[i], a[i], b[i]);
        }
    }
    
    static inline void _add_vec(T*c, const T* a, const T* b, size_t n) noexcept {
        _vec_impl(c, a, b, n, _add_op_t{});
    }

    static inline void _sub_vec(T* c, const T* a, const T* b, size_t n) noexcept {
        _vec_impl(c, a, b, n, _sub_op_t{});
    }

    static inline void _mul_vec(T* c, const T* a, const T* b, size_t n) noexcept {
        _vec_impl(c, a, b, n, _mul_op_t{});
    }

    static inline void _div_vec(T* c, const T* a, const T* b, size_t n) noexcept {
        _vec_impl(c, a, b, n, _div_op_t{});
    }

    static inline void _add_scalar(T* c, const T* a, const T& b, size_t n) noexcept {
        for (size_t i = 0; i < n; i++) {
            c[i] = a[i] + b;
        }
    }

    static inline void _sub_scalar(T* c, const T* a, const T& b, size_t n) noexcept {
        for (size_t i = 0; i < n; i++) {
            c[i] = a[i] - b;
        }
    }

    static inline void _mul_scalar(T* c, const T* a, const T b, size_t n) noexcept {
        for (size_t i = 0; i < n; i++) {
            c[i] = a[i] * b;
        }
    }

    static inline void _div_scalar(T* c, const T* a, const T b, size_t n) noexcept {
        for (size_t i = 0; i < n; i++) {
            c[i] = a[i] * b;
        }
    }
};
    
#if __AVX2__

template<>
struct arith<double> {

    using BaseType = double;
    using RegType = __m256d;
    static constexpr size_t Alignment = sizeof(RegType);
    static constexpr size_t OpCapacity = (sizeof(RegType) / sizeof(BaseType));

    static_assert(OpCapacity == 4, "Bad assumption with size of SIMD registers");

    struct _add_op_t {
        static inline void _exec(__m256d& c, const __m256d& a, const __m256d& b) noexcept {
            c = _mm256_add_pd(a, b);
        }
    };
    struct _sub_op_t {
        static inline void _exec(__m256d& c, const __m256d& a, const __m256d& b) noexcept {
            c = _mm256_sub_pd(a, b);
        }
    };
    struct _mul_op_t {
        static inline void _exec(__m256d& c, const __m256d& a, const __m256d& b) noexcept {
            c = _mm256_mul_pd(a, b);
        }
    };
    struct _div_op_t {
        static inline void _exec(__m256d& c, const __m256d& a, const __m256d& b) noexcept {
            c = _mm256_div_pd(a, b);
        }
    };

    private:
    // Few notes:
    // We will assume that all array sizes are divisible by 4
    // This will be guaranteed by the vec class, which will provide 
    // the necessary padding at the end of its array
    
    template <typename Op, typename Intrin = __m256d> requires VecOp<Op, Intrin>
    static inline void _vec_impl(double* c, const double* a, const double* b, size_t n, Op) noexcept {
        ASSERT(autil::_assert_align<Alignment>(a, b));
        __m256d _a, _b, _c;
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            _a = _mm256_load_pd(&a[offset]); 
            _b = _mm256_load_pd(&b[offset]);
            Op::_exec(_c, _a, _b);
            _mm256_store_pd(&c[offset], _c);
        }     
    }

    template <typename Op, typename ...Args, typename Intrin = __m256d> requires VecOp<Op, Intrin>
    static inline void _vec_impl(Op, size_t n, double* out, Args*... args) noexcept {
        ASSERT(autil::_assert_align<Alignment>(out, args...));
        __m256d _out;
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            Op::_exec(_out, _mm256_load_pd(&args[offset])...);
            _mm256_store_pd(&out[offset], _out);
        }     
    }

    template<typename Op, typename Intrin = __m256d> requires VecOp<Op, Intrin>
    static inline void _scalar_impl(Op, size_t n, double* c, const double* a, const double& b) noexcept {
        ASSERT(autil::_assert_align<Alignment>(a));
        __m256d _b = _mm256_broadcast_sd(&b);
        for (uint32_t i = 0; i < autil::_num_loops<OpCapacity>(n); i++) {
            uint32_t offset = OpCapacity * i;
            __m256d _a = _mm256_load_pd(&a[offset]); 
            __m256d _c; 
            Op::_exec(_c, _a, _b);
            _mm256_store_pd(&c[offset], _c);
        }
    }

    public:
    static inline void _add_vec(double* c, const double* a, const double* b, size_t n) noexcept {
        _vec_impl(_add_op_t{}, n, c, a, b);
    }

    static inline void _sub_vec(double* c, const double* a, const double* b, size_t n) noexcept {
        _vec_impl(_sub_op_t{}, n, c, a, b);
    }

    static inline void _mul_vec(double* c, const double* a, const double* b, size_t n) noexcept {
        _vec_impl(_mul_op_t{}, n, c, a, b);
    }

    static inline void _div_vec(double* c, const double* a, const double* b, size_t n) noexcept {
        _vec_impl(_div_op_t{}, n, c, a, b);
    }

    static inline void _add_scalar(double* c, const double* a, const double& b, size_t n) noexcept {
        _scalar_impl(_add_op_t{}, n, c, a, b);
    }

    static inline void _sub_scalar(double* c, const double* a, const double& b, size_t n) noexcept {
        _scalar_impl(_sub_op_t{}, n, c, a, b);
    }

    static inline void _mul_scalar(double* c, const double* a, const double& b, size_t n) noexcept {
        _scalar_impl(_mul_op_t{}, n, c, a, b);
    }

    static inline void _div_scalar(double* c, const double* a, const double& b, size_t n) noexcept {
        _scalar_impl(_div_op_t{}, n, c, a, b);
    }

};
#endif 

using farith = arith<float>;
using darith = arith<double>;
