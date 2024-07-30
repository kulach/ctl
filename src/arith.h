#pragma once

#include <arith/basearith.h>
#include <arith/sarith.h>
#include <arith/carith.h>
#include <complex.h>

template <typename T>
struct arith_type_impl {
    using Arith = arith<T>;
};

template <typename T>
struct arith_type_impl<complex<T>> {
    using Arith = carith<complex<T>>;
};

template <typename T>
using Arith = arith_type_impl<T>::Arith;
