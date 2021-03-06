#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <nonstd/optional.hpp>

#include "chainerx/array.h"
#include "chainerx/array_index.h"
#include "chainerx/axes.h"
#include "chainerx/device.h"
#include "chainerx/dtype.h"
#include "chainerx/graph.h"
#include "chainerx/op.h"
#include "chainerx/scalar.h"
#include "chainerx/shape.h"

namespace chainerx {
namespace internal {

// Returns the minimum number of bytes required to pack the data with specified strides and shape.
// TODO(niboshi): Replace this with chainerx::GetDataRange()
size_t GetRequiredBytes(const Shape& shape, const Strides& strides, size_t item_size);

// Creates an array with given data packed with specified strides
Array FromHostData(
        const Shape& shape,
        Dtype dtype,
        const std::shared_ptr<void>& data,
        const Strides& strides,
        int64_t offset,
        Device& device = GetDefaultDevice());

// Creates an empty array with specified strides.
Array Empty(const Shape& shape, Dtype dtype, const Strides& strides, Device& device = GetDefaultDevice());

// Creates an empty array with reduced shape.
Array EmptyReduced(const Shape& shape, Dtype dtype, const Axes& axes, bool keepdims, Device& device = GetDefaultDevice());

}  // namespace internal

// Creates an array with given contiguous data
Array FromContiguousHostData(const Shape& shape, Dtype dtype, const std::shared_ptr<void>& data, Device& device = GetDefaultDevice());

// Creates an array with given data without copying.
//
// The data must reside in the specified device.
// If strides are not given, the data is considered as a contiguous data.
Array FromData(
        const Shape& shape,
        Dtype dtype,
        const std::shared_ptr<void>& data,
        const nonstd::optional<Strides>& strides = nonstd::nullopt,
        int64_t offset = 0,
        Device& device = GetDefaultDevice());

class ArangeOp : public Op {
public:
    static const char* name() { return "Arange"; }

    virtual void Call(Scalar start, Scalar step, const Array& out) = 0;
};

class CopyOp : public Op {
public:
    static const char* name() { return "Copy"; }

    // Copies the elements from one array to the other.
    //
    // The arrays must match in shape and dtype and need to reside on this device.
    virtual void Call(const Array& a, const Array& out) = 0;
};

class IdentityOp : public Op {
public:
    static const char* name() { return "Identity"; }

    // Creates the identity array.
    // out must be a square 2-dim array.
    virtual void Call(const Array& out) = 0;
};

class EyeOp : public Op {
public:
    static const char* name() { return "Eye"; }

    // Creates a 2-dimensional array with ones along the k-th diagonal and zeros elsewhere.
    // out must be a square 2-dim array.
    virtual void Call(int64_t k, const Array& out) = 0;
};

class DiagflatOp : public Op {
public:
    static const char* name() { return "Diagflat"; }

    virtual void Call(const Array& v, int64_t k, const Array& out) = 0;
};

class LinspaceOp : public Op {
public:
    static const char* name() { return "Linspace"; }

    // Creates an evenly spaced 1-d array.
    // `out.ndim()` must be 1 with at least 1 elements.
    virtual void Call(double start, double stop, const Array& out) = 0;
};

Array Empty(const Shape& shape, Dtype dtype, Device& device = GetDefaultDevice());
Array Full(const Shape& shape, Scalar fill_value, Dtype dtype, Device& device = GetDefaultDevice());
Array Full(const Shape& shape, Scalar fill_value, Device& device = GetDefaultDevice());
Array Zeros(const Shape& shape, Dtype dtype, Device& device = GetDefaultDevice());
Array Ones(const Shape& shape, Dtype dtype, Device& device = GetDefaultDevice());

Array Arange(Scalar start, Scalar stop, Scalar step, Dtype dtype, Device& device = GetDefaultDevice());
Array Arange(Scalar start, Scalar stop, Scalar step, Device& device = GetDefaultDevice());
Array Arange(Scalar start, Scalar stop, Dtype dtype, Device& device = GetDefaultDevice());
Array Arange(Scalar start, Scalar stop, Device& device = GetDefaultDevice());
Array Arange(Scalar stop, Dtype dtype, Device& device = GetDefaultDevice());
Array Arange(Scalar stop, Device& device = GetDefaultDevice());

// Creates an array which has the same shape and dtype as the other array.
// The new array is allocated in the default device. The device of the other array
// is ignored.
Array EmptyLike(const Array& a, Device& device = GetDefaultDevice());
Array FullLike(const Array& a, Scalar fill_value, Device& device = GetDefaultDevice());
Array ZerosLike(const Array& a, Device& device = GetDefaultDevice());
Array OnesLike(const Array& a, Device& device = GetDefaultDevice());

// Creates a copy.
// It will be connected to all the graphs.
// It will be always C-contiguous.
Array Copy(const Array& a);

// Creates the identity array.
Array Identity(int64_t n, Dtype dtype, Device& device = GetDefaultDevice());

// Creates a 2-dimensional array with ones along the k-th diagonal and zeros elsewhere.
Array Eye(
        int64_t n,
        nonstd::optional<int64_t> m,
        nonstd::optional<int64_t> k,
        nonstd::optional<Dtype> dtype,
        Device& device = GetDefaultDevice());

namespace internal {

// Returns a C-contiguous array without changing input shape.
Array AsContiguous(const Array& a, Dtype dtype);

// Returns a C-contiguous array with the same shape and dtype as the input array.
inline Array AsContiguous(const Array& a) { return AsContiguous(a, a.dtype()); }

}  // namespace internal

// Returns a C-contiguous array.
// An input array with shape {} results in a new array with shape {1}.
Array AsContiguousArray(const Array& a, const nonstd::optional<Dtype>& dtype = nonstd::nullopt);

// TODO(niboshi): Remove device argument and use v.device(). Also fix tests
Array Diag(const Array& v, int64_t k = 0, Device& device = GetDefaultDevice());

// TODO(niboshi): Remove device argument and use v.device(). Also fix tests
Array Diagflat(const Array& v, int64_t k = 0, Device& device = GetDefaultDevice());

// Creates a 1-d array with evenly spaced numbers.
Array Linspace(
        Scalar start,
        Scalar stop,
        const nonstd::optional<int64_t>& num = nonstd::nullopt,
        bool endpoint = true,
        const nonstd::optional<Dtype>& dtype = nonstd::nullopt,
        Device& device = GetDefaultDevice());

}  // namespace chainerx
