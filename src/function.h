#pragma once

#include <vec.h>
#include <memory>
#include <vector>
#include <cmath>
#include <tview.h>
#include <ranges>
#include <algorithm>

// This class will serve as the interface to generalize all 
// function types, it will have two necessary 
template<typename T> 
class BaseFunction {

    public:
    virtual MutView<T> operator()(MutView<T> input) const = 0;
    [[deprecated]]
    virtual size_t input_size() const = 0;
    virtual ~BaseFunction() {};

};

//template<typename T>
//concept FunctionReq = requires {
//    requires std::derived_from<T, BaseFunction>;
//    {T::InputSize} -> std::convertible_to<const size_t>;
//};


// Composition Function is the mother of all functions
// It can hold many functions in a tree-like format
// This format officially will merely be a 
template <typename T>
class CompositeFunction : public BaseFunction<T> {

    using FuncPtr = std::shared_ptr<BaseFunction<T>>;
    // The last part of this sequence will be the identity function containing the input
    std::vector<FuncPtr> funcs;

    public:
    CompositeFunction() {
    };

    // g(x) -> f(g(x))
    void compose_outer(FuncPtr outer) {
        if (auto ptr = std::dynamic_pointer_cast<CompositeFunction>(outer)) {
            // Steal the current functions in the input composite function
            // This has the strange side effect that declares that internal composite
            // functions cannnot change
            auto& new_funcs = ptr->get_funcs();
            for (auto func : new_funcs) {
                funcs.emplace_back(std::move(func));
            }
        } else {
            funcs.emplace_back(std::move(outer));
        }
    }

    MutView<T> operator()(MutView<T> input) const override {
        // Run the sequence
        auto data = input;
        for (auto func : funcs) {
            data = func->operator()(data);
        }
        return data;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    const auto& get_funcs() const noexcept {
        return funcs;
    }
};

// Do nothing
template <typename T>
class IdentityFunction : virtual BaseFunction<T> {

    public:
    IdentityFunction() {
    }

    MutView<T> operator()(MutView<T> input) const override {
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }
};

// Takes a Vec and returns Vec[index]
template <typename T>
class MapFunction : public BaseFunction<T> {
    Vec<T> lookup;
    public:
    MapFunction(Vec<T> lookup) : lookup(std::move(lookup)) {
    }

    MutView<T> operator()(MutView<T> input) const override {
        double* in_arr = input.data();
        double* lookup_arr = lookup.data();
        for (size_t i = 0; i < input.size(); i++) {
            size_t index = std::round(in_arr[i]);
            in_arr[i] = lookup_arr[index];
        }
        return input;
    }

    constexpr size_t input_size() const override {
        return lookup.ndim();
    }

};

// Multiply everything by a
template <typename T>
class ScalerFunction : public BaseFunction<T> {
    T _scale;
    public: 
    ScalerFunction(T scale) : _scale(scale) {
    }

    MutView<T> operator()(MutView<T> input) const override {
        input *= _scale;
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    constexpr const T& scale() const noexcept {
        return _scale;
    }
};

// Add everything by a
template <typename T>
class AddFunction : public BaseFunction<T> {
    T delta;
    public:
    AddFunction(T delta) : delta(delta) {
    }

    MutView<T> operator()(MutView<T> input) const override {
        input += delta;
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    } 
};


// This function will take a vector of R^N, then shift its inputs to the right by k
// Meaning R_i = R_(i-k)
// This is akin to f(x) -> f(x - k)
template <typename T> 
class ShiftFunction : public BaseFunction<T> {
    int64_t _shift;
    public:
    ShiftFunction(int64_t shift) : _shift(shift) {
    }

    MutView<T> operator()(MutView<T> input) const override {
        // This is an expensive operation, mainly from the awkward memory usage
        // Uses the std::rotate function with 

        // std::ranges::rotate(input, input.begin() + (_shift % input.size()));
        int64_t s = (-_shift) % static_cast<uint64_t>(input.size());
        std::ranges::rotate(input, input.begin() + s);
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    constexpr size_t shift() const {
        return _shift;
    }
};

template<typename T>
class SumFunction : public BaseFunction<T> {
    Vec<T> summand;
    ConstView<T> sview;

    public:
    SumFunction(Vec<T> summand) : summand(std::move(summand)), sview(this->summand) {

    }

    MutView<T> operator()(MutView<T> input) const override {
        input += sview;
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    ConstView<T> get_addend() const {
        return sview;
    }
};

template<typename T>
class MultFunction : public BaseFunction<T> {
    Vec<T> multiplicand;
    ConstView<T> mview;

    public:
    MultFunction(Vec<T> multiplicand) : multiplicand(std::move(multiplicand)), mview(this->multiplicand) {
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    MutView<T> operator()(MutView<T> input) const override {
        input *= mview;
        return input;
    }
};

template<typename T>
class ProductFunction : public BaseFunction<T> {
    Vec<T> multiplicand;
    ConstView<T> mview;

    public:
    ProductFunction(Vec<T> multiplicand) : multiplicand(std::move(multiplicand)), mview(this->multiplicand) {

    }

    MutView<T> operator()(MutView<T> input) const override {
        input *= mview;
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

    ConstView<T> get_multiplicand() const {
        return multiplicand;
    }
};

// Returns the conjugation of the input aka
// a + jb -> a - jb
template<typename T> requires ComplexType<T>
class ConjFunction : public BaseFunction<T> {
    public: 

    ConjFunction() {}

    MutView<T> operator()(MutView<T> input) const override {
        for (auto c : input) {
            c.im = -c.im;
        }
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }
};

template<typename T> 
class CircularReverse : public BaseFunction<T> {
    public:
    CircularReverse() {}

    MutView<T> operator()(MutView<T> input) const override {
        // Circular reverse keeps the 0 index the same
        // Therefore, anything less than 2 is not necessary
        if (input.size() > 2) [[likely]] {
            std::ranges::reverse(input.begin() + 1, input.end());
        }
        return input;
    }

    constexpr size_t input_size() const override {
        return 1;
    }

};
