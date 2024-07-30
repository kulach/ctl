# CTL

CTL - Complex Transform Library

## Introduction

CTL is meant to provide a framework for complex arithmetic transformations

Its principal features include: 
 - Vector Arithmetic of Scalar and Complex Types
 - AVX2 SIMD Instruction Arithmetic for double sized floating points (AMD64 architecture)
 - Vector Functions
 - Function Composition
 - Fourier Transformation Dual Support
 - Random Access Iterator and Range Support
 - STL Algorithm Support

## Compilation

The default provided binary (stemmed from ctl.cpp) contains a benchmark that is meant
to calculate the time to calculate a number of complex FFTs using the library.

Compilation will require a compiler with C++20 support.

To compile, using the default configuration, use the command:

`~$ cmake -DCMAKE_BUILD_TYPE=Release .`

To compile with AVX2 support use the command:

`~$ cmake -DCMAKE_BUILD_TYPE=Release -DAVX2=TRUE .`

A list of compilation options is shown below:

(The first element in the options column is the default)

| Variable | Options | Description|
|:---------|:--------|:-----------|
|CMAKE_BUILD_TYPE| Release, Debug| Sets the optimization level (Debug: -g), (Release: -O3)|
|ADDRESS_SANITIZER| FALSE, TRUE| Enables the address sanitizer by using -fsanitize=address (ASAN must be installed)|
|PERF| FALSE, TRUE| Enables flags that allow the use of perf record on Linux (Enables -g -fno-omit-frame-pointer)|
|AVX2| FALSE, TRUE| Enables AVX2 instructions, if this is disabled, the library will default to using single floating point operations|

Compilation will provide two binaries: ctl and ctltests. The first will be the benchmark as mentioned above, 
while the second will be a suite of tests using the [utest framework by sheredom](https://github.com/sheredom/utest.h).

## Use case

By default, there are a few functions that are included in the library. The current set of functions 
have been tailored to be able to be able to have corresponding fourier dual transfomation functions. 
For example, given a typical fourier pair f\[i\] <-> F\[k\] where f is the original complex signal in discrete points,
and F is its corresponding Discrete Fourier Transform. Given some function u: R^n -> R^n, we may be able to construct 
a corresponding v: R^n -> R^n such that u(f\[i\]) <-> v(F\[k\]) and vice versa. 

Such functions exist, and a few have been supported in the FourierDual class. Examples include scaling, convolution, and multipliation.

This FourierDual class (and other corresponding transformations that can use such a pattern), can be used to avoid potentially expensive 
transformations where the input is passed through a trivial function u() multiple times, and an output must be calculated for multiple 
of these functions. For example, in the simple case where we must rotate the input sequence by some n, we can do so by

```cpp
const size_t size = 16; 
FourierDual<double> fourier(size);  

// Composition function object to hold compositions of functions 
CompositeFunction<complex<double>> comp;                    

// Shift the input by 7
auto shift = std::make_shared<ShiftFunction<cplx128_t>>(7); 

// Create a composition c(x) = u(x)
comp.compose_outer(shift);                                  

// Set the time input (u) to be the function responsible for rotation, correspodingly, this will create function v
fourier.set_time_func(comp);                                
```

This setup will now allow us to use two functions:
` fourier.u(x);`
Which will be the original shift equation, that rotates the input by 7, and
` fourier.v(X);`
Which will perform an elementwise multiplication to the input in frequency space by exp(-2 * pi * 7 * i / N)

Other examples include Scalar multiplication, summation, conjugation, and elementwise multiplication.
