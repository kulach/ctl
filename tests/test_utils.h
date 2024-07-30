#pragma once

#include "common.h"
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <random>
#include <complex.h>

namespace tutil {

    template<typename T> requires FloatingType<T>
    constexpr bool eq(const T& a, const T& b) {
        constexpr T EPS = .001;
        double c = a - b;
        return (c < EPS && c > -EPS);
    }

    template<typename T> requires IntegerType<T>
    constexpr bool eq(const T& a, const T& b) {
        return a == b;
    }

    inline bool eq(complexref<double> a, complexref<double> b) {
        constexpr double EPS = .001;
        double cr = a.re - b.re;
        double ci = a.im - b.im;
        return (cr < EPS && cr > -EPS && ci < EPS && ci > -EPS);
    }

    inline bool eq(ccomplexref<double> a, ccomplexref<double> b) {
        constexpr double EPS = .001;
        double cr = a.re - b.re;
        double ci = a.im - b.im;
        return (cr < EPS && cr > -EPS && ci < EPS && ci > -EPS);
    }

    template <typename A, typename B> requires ComplexType<A> && ComplexType<B>
    inline bool ceq(A a, B b) {
        return eq(complexref(a), complexref(b));
    }

    inline bool deq(double a, double b) {
        return eq(a, b);
    }

    template <typename T, typename Ptr>
    inline bool random_check(Ptr arr, size_t size, std::function<T(size_t)> expected) {
        constexpr auto CHECKS = 16;
        std::random_device r;
        std::default_random_engine engine{r()};
        std::uniform_int_distribution<size_t> gen(0, size-1);

        for (auto i = 0; i < CHECKS; i++) {
            size_t index = gen(engine);
            auto a = arr[index];
            T b = expected(index);
            if (!eq(a, b)) {
                std::cout << "Found unequal check in random check: index: (expected, real): " << index << ": (" << b << ", " << a << ")\n";
                return false;
            }
        }
        return true;
    }

    template <typename Ptr>
    inline bool random_eq(Ptr a, Ptr b, size_t size) {
        constexpr auto CHECKS = 16;
        std::random_device r;
        std::default_random_engine engine{r()};
        std::uniform_int_distribution<size_t> gen(0, size-1);

        for (auto i = 0; i < CHECKS; i++) {
            size_t index = gen(engine);
            auto _a = a[index];
            auto _b = b[index];
            if (!eq(_a, _b)) {
                std::cout << "Found unequal check in random check: index: (expected, real): " << index << ": (" << _b << ", " << _a << ")\n";
                return false;
            }
        }
        return true;
    }
 
};
