// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ARRAY_HELPERS__HPP__
#define __AIE_API_DETAIL_ARRAY_HELPERS__HPP__

#include <cstddef>
#include <type_traits>

#include "vector.hpp"
#if AIE_API_ML_VERSION >= 210
#include "../block_vector.hpp"
#endif
#include "../sparse_vector.hpp"
#include "../vector_elem_ref.hpp"

// ADF declarations
template <typename T> struct input_stream;
template <typename T> struct output_stream;

namespace aie {

inline constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

namespace detail {

enum class tbs_mode
{
    default_mode,
    weight_read,
    activation_read
};

enum class data_layout
{
    row_major,
    column_major
};

template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc, data_layout Layout,
          typename ResourceType, ResourceType Resource, tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U>
struct tbs;

} // namespace detail

template <typename T> struct is_tbs : std::false_type {};

template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc, detail::data_layout Layout,
          typename ResourceType, ResourceType Resource, detail::tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U>
struct is_tbs<detail::tbs<T, Elems, Level, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>> : std::true_type {};

}

namespace aie::detail {

template <typename T>
struct get_value_type
{
    using type = T;
};

template <typename T, unsigned Elems>
struct get_value_type<vector<T, Elems>>
{
    using type = typename vector<T, Elems>::value_type;
};

template <typename T>
using get_value_type_t = typename get_value_type<T>::type;

template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource = aie_dm_resource::none>
    requires (arch::is(arch::Gen2))
class sliding_window_buffer_stream;

template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource = aie_dm_resource::none>
    requires (arch::is(arch::Gen2))
using          const_sliding_window_buffer_stream = sliding_window_buffer_stream<const std::remove_const_t<T>, Elems, IterDescriptor, Resource>;

#if AIE_API_ML_VERSION >= 200
template <bool ByteIncr = true>
struct multidim_incr
{
    template <typename T, typename IncrType>
    __aie_inline
    static T* run(T *ptr, IncrType& dim) {
        if constexpr (ByteIncr) {
            if      constexpr (std::is_same_v<IncrType, ::dims_3d_t>) return ::add_3d_byte(ptr, dim);
            else if constexpr (std::is_same_v<IncrType, ::dims_2d_t>) return ::add_2d_byte(ptr, dim);
            else                                                      return ::byte_incr(ptr, dim);
        }
        else {
            if      constexpr (std::is_same_v<IncrType, ::dims_3d_t>) return ::add_3d_ptr(ptr, dim);
            else if constexpr (std::is_same_v<IncrType, ::dims_2d_t>) return ::add_2d_ptr(ptr, dim);
            else                                                      return ptr + dim;
        }
    }
};
#endif

struct dim_2d
{
    unsigned num1; int inc1; int inc2;

    constexpr dim_2d() = default;

    constexpr dim_2d(unsigned num1, int inc1, int inc2) : num1(num1), inc1(inc1), inc2(inc2) {}
#if AIE_API_ML_VERSION > 100
    constexpr dim_2d(const ::dims_2d_t& d) : num1(d.num1), inc1(d.inc1), inc2(d.inc2) {}
#endif
};

struct dim_3d
{
    unsigned num1; int inc1; unsigned num2; int inc2; int inc3;

    constexpr dim_3d() = default;

    constexpr dim_3d(unsigned num1, int inc1, unsigned num2, int inc2, int inc3) : num1(num1), inc1(inc1), num2(num2), inc2(inc2), inc3(inc3) {}
#if AIE_API_ML_VERSION > 100
    constexpr dim_3d(const ::dims_3d_t& d) : num1(d.num1), inc1(d.inc1), num2(d.num2), inc2(d.inc2), inc3(d.inc3) {}
#endif
};

struct sliding_window_dim_1d
{
    int inc1;

    constexpr sliding_window_dim_1d() = default;

    constexpr sliding_window_dim_1d(int step) :
#if __AIE_ARCH__ == 20
        inc1(step)
#else
        // unaligned loads via the fifo advances the input pointer by 64B
        inc1(-64 + step)
#endif
    {}

    operator int() { return inc1; }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

struct sliding_window_dim_2d
{
    unsigned num1; int inc1; int inc2;

    constexpr sliding_window_dim_2d() = default;

    constexpr sliding_window_dim_2d(unsigned num1, int step,
                                                   int inc2) :
#if __AIE_ARCH__ == 20
        num1(num1), inc1(step),       inc2(step + inc2)
#else
        // unaligned loads via the fifo advances the input pointer by 64B
        num1(num1), inc1(-64 + step), inc2(-64 + step + inc2)
#endif
    {}

    explicit constexpr sliding_window_dim_2d(const dim_2d& other) :
        sliding_window_dim_2d(other.num1, other.inc1, other.inc2) {}

    constexpr sliding_window_dim_2d(const sliding_window_dim_2d& other) = default;

    operator dim_2d() { return dim_2d(num1, inc1, inc2); }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

struct sliding_window_dim_3d
{
    unsigned num1; int inc1; unsigned num2; int inc2; int inc3;

    constexpr sliding_window_dim_3d() = default;

    constexpr sliding_window_dim_3d(unsigned num1, int step,
                                    unsigned num2, int inc2,
                                                   int inc3) :
#if __AIE_ARCH__ == 20
        num1(num1), inc1(step),       num2(num2), inc2(step + inc2),       inc3(step + inc3)
#else
        // unaligned loads via the fifo advances the input pointer by 64B
        num1(num1), inc1(-64 + step), num2(num2), inc2(-64 + step + inc2), inc3(-64 + step + inc3)
#endif
    {}

    explicit constexpr sliding_window_dim_3d(const dim_3d& other) :
        sliding_window_dim_3d(other.num1, other.inc1, other.num2, other.inc2, other.inc3) {}

    constexpr sliding_window_dim_3d(const sliding_window_dim_3d& other) = default;

    operator dim_3d() { return dim_3d(num1, inc1, num2, inc2, inc3); }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

// A contiguous dimension is a special case of a 1D tensor_dim with step equal to the number of elements in the vector type.
// It is used to represent the innermost dimension of a tensor that is contiguous in memory.
struct contiguous_dim {};

template <typename... Args>
struct compute_rank
{
    template <typename T>
    struct compute_rank_helper
    {
        using U = std::decay_t<T>;
        static constexpr unsigned value = utils::is_one_of_v<U, dim_3d, sliding_window_dim_3d> ? 3
                                        : utils::is_one_of_v<U, dim_2d, sliding_window_dim_2d> ? 2
                                        : 1;
    };
    static constexpr unsigned value = (compute_rank_helper<Args>::value + ...);
};

template <typename... Args>
struct compute_rank<std::tuple<Args...>>
{
    static constexpr unsigned value = compute_rank<Args...>::value;
};

template <typename... Args>
static constexpr unsigned compute_rank_v = compute_rank<Args...>::value;

template <unsigned N> struct default_repr;
template <>           struct default_repr<0> { using type = std::tuple<>; };
template <>           struct default_repr<1> { using type = std::tuple<int>; };
template <>           struct default_repr<2> { using type = std::tuple<dim_2d>; };
template <unsigned N> struct default_repr    { using type = decltype(std::tuple_cat(std::declval<std::tuple<dim_3d>>(),
                                                                                    std::declval<typename default_repr<N-3>::type>())); };
template <unsigned N> using default_repr_t = typename default_repr<N>::type;

template <typename U>
struct iter_state
{
    struct state_1d {};
    struct state_2d { addr_t c1 = 0; };
    struct state_3d { addr_t c1 = 0, c2 = 0; };
    using storage_t = std::conditional_t<utils::is_one_of_v<U, dim_3d, sliding_window_dim_3d>,
                                        state_3d,
                                        std::conditional_t<utils::is_one_of_v<U, dim_2d, sliding_window_dim_2d>,
                                                          state_2d,
                                                          state_1d>>;
    storage_t state_;
};

#if AIE_API_ML_VERSION >= 210
template <typename T, unsigned Elems, bool IsBlock> struct tensor_desc_type;
template <typename T, unsigned Elems> struct tensor_desc_type<T, Elems, false> { using type = vector<T, Elems>;     };
template <typename T, unsigned Elems> struct tensor_desc_type<T, Elems, true>  { using type = block_vector<T, Elems>; };
template <typename T>                 struct tensor_desc_type<T,    1u, false> { using type = T; };
template <Mask T,     unsigned Elems> struct tensor_desc_type<T, Elems, false> { using type = T; };
template <typename T, unsigned Elems> using tensor_desc_type_t = typename tensor_desc_type<T, Elems, is_valid_block_type_v<T>>::type;
#else
template <typename T, unsigned Elems> struct tensor_desc_type           { using type = vector<T, Elems>; };
template <typename T>                 struct tensor_desc_type<T, 1u>    { using type = T; };
template <Mask T,     unsigned Elems> struct tensor_desc_type<T, Elems> { using type = T; };
template <typename T, unsigned Elems> using tensor_desc_type_t = typename tensor_desc_type<T, Elems>::type;
#endif

#if AIE_API_ML_VERSION >= 200

// helpers to disambiguate specializations
template <typename T> struct is_stream                   : std::false_type {};
template <typename T> struct is_stream<input_stream<T>>  : std::true_type  {};
template <typename T> struct is_stream<output_stream<T>> : std::true_type  {};

// Structure for input stream + memory pointer
template <typename T, typename ElemType>
    requires std::is_same_v<T, input_stream<ElemType>>
struct stream_with_mem_ptr {
    T* stream_ptr;
    ElemType* mem_ptr;
};

// Type trait to detect stream_with_mem_ptr
template <typename T> struct is_stream_with_mem_ptr : std::false_type {};
template <typename StreamType, typename ElemType>
struct is_stream_with_mem_ptr<stream_with_mem_ptr<StreamType, ElemType>> : std::true_type {};

template <typename T> struct io_buffer_storage {};
template <IOBuffer T> struct io_buffer_storage<T>
{
    T &port_;

    io_buffer_storage(T &port) : port_(port) {}

    auto& port() { return port_; }
};

// pointer/io_buffer specialization
template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc,
          typename ResourceType, ResourceType Resource, tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U>
struct tbs_impl
{
    using          elem_type = T;
    using        vector_type = tensor_desc_type_t<std::remove_const_t<elem_type>, Elems>;
    using         value_type = add_memory_bank_t<Resource, vector_type>;
    using            pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using        iter_desc_t = std::decay_t<IterDesc>;
    using  iter_desc_storage = iter_desc_t;
    using iter_state_storage = iter_state<std::tuple_element_t<Level, iter_desc_t>>;
    using accum_tag = detail::accum_tag_for_type<elem_type, 32>;

    static constexpr bool next_sliding    = Level == NumLevels - 2 && SlidingInner;
    static constexpr bool innermost_level = Level == NumLevels - 1;
    static constexpr bool is_const = std::is_const_v<U>;

    using inner_type = std::conditional_t<innermost_level,
                                          vector_type,
                                          std::conditional_t<next_sliding,
                                                             sliding_window_buffer_stream<T, Elems, std::tuple_element_t<NumLevels-1, iter_desc_t>, Resource>,
                                                             tbs_impl<T, Elems, Level + 1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, Unaligned, U>>>;

    __aie_inline
    constexpr tbs_impl(U *ptr, const IterDesc& iter_desc) :
        ptr_(ptr), iter_desc_(iter_desc) {}

    __aie_inline
    constexpr tbs_impl(U &port, const IterDesc& iter_desc) requires (IOBuffer<U>) :
        ptr_(port.data()), iter_desc_(iter_desc), port_(port) {}

    __aie_inline
    constexpr inner_type pop()
    {
        if constexpr (innermost_level) {
            inner_type v;
            if constexpr (Elems == 1) {
                v = *ptr_;
            }
            else {
                v.load(utils::floor_ptr<Elems>(ptr_));
            }
            increment();
            return v;
        }
        else if constexpr (next_sliding) {
            inner_type v = inner_type(ptr_, std::get<NumLevels - 1>(iter_desc_));
            increment();
            return v;
        }
        else {
            inner_type v = inner_type(*this);
            increment();
            return v;
        }
    }

    __aie_inline
    constexpr void push(const accum<accum_tag, Elems>& acc, int shift = 0) requires (innermost_level && Elems > 1)
    {
        push(acc.template to_vector<elem_type>(shift));
    }

    __aie_inline
    constexpr void push(const vector_type& v) requires (innermost_level)
    {
        *(pointer)ptr_ = v;
        increment();
    }

    __aie_inline
    void acquire()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().acquire();
            ptr_ = port_.port().data();
        }
        return;
    }

    __aie_inline
    void release()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().release();
        }
        return;
    }

#if AIE_API_NATIVE
    using ptr_type = std::conditional_t<is_const, const T*, T*>;
#else
    using ptr_type = std::conditional_t<is_const,
                                        std::conditional_t<Restrict, const T* __restrict, const T*>,
                                        std::conditional_t<Restrict,       T* __restrict,       T*>>;
#endif

    __aie_inline
    constexpr void increment(void)
    {
        const auto& inc = std::get<Level>(iter_desc_);
        if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_3d_byte(ptr_, inc.inc3,
                                       inc.num1, iter_state_.state_.c1, inc.inc1,
                                       inc.num2, iter_state_.state_.c2, inc.inc2);
        }
        else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_2d_byte(ptr_, inc.inc2,
                                       inc.num1, iter_state_.state_.c1, inc.inc1);
        }
        else if constexpr(std::is_same_v<contiguous_dim, std::decay_t<decltype(inc)>>) {
            ptr_ += Elems;
        }
        else {
            ptr_ = ::byte_incr(ptr_, inc);
        }
    }

    // Internal constructors for creating inner streams
    using outer_type = tbs_impl<T, Elems, Level == 0 ? 0 : Level-1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, Unaligned, U>;

    __aie_inline
    constexpr tbs_impl(const outer_type& other) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_) {}

    __aie_inline
    constexpr tbs_impl(const outer_type& other) requires (IOBuffer<U>) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_), port_(other.port_) {}


    ptr_type ptr_;
    iter_desc_storage iter_desc_;
    iter_state_storage iter_state_;
    io_buffer_storage<U> port_;
};

// pointer/io_buffer specialization for masks
template <Mask T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc,
          typename ResourceType, ResourceType Resource, tbs_mode Mode,
          bool Restrict,
          typename U>
struct tbs_impl<T, Elems, Level, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>
{
    using native_type = std::conditional_t<Elems == 8,  uint8,
                        std::conditional_t<Elems == 16, uint16,
                        std::conditional_t<Elems == 32, uint32,
                        std::conditional_t<Elems == 64, mask64, mask64>>>>;

    using          elem_type = T;
    using        vector_type = T;
    using         value_type = add_memory_bank_t<Resource, elem_type>;
    using            pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using        iter_desc_t = std::decay_t<IterDesc>;
    using  iter_desc_storage = iter_desc_t;
    using iter_state_storage = iter_state<std::tuple_element_t<Level, iter_desc_t>>;

    static constexpr bool next_sliding    = false;
    static constexpr bool innermost_level = Level == NumLevels - 1;
    static constexpr bool is_const = std::is_const_v<U>;

    using inner_type = std::conditional_t<innermost_level,
                                          vector_type,
                                          tbs_impl<T, Elems, Level + 1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>>;

    __aie_inline
    constexpr tbs_impl(U *ptr, const IterDesc& iter_desc) :
        ptr_((native_type*)ptr), iter_desc_(iter_desc) {}

    __aie_inline
    constexpr tbs_impl(U &port, const IterDesc& iter_desc) requires (IOBuffer<U>) :
        ptr_((native_type*)(port.data())), iter_desc_(iter_desc), port_(port) {}

    __aie_inline
    constexpr inner_type pop()
    {
        if constexpr (innermost_level) {
            inner_type v;
            if constexpr (Elems <= 32)
                v = inner_type::from_uint32(*utils::floor_ptr<Elems>(ptr_));
            else if constexpr (Elems == 64)
                v = inner_type::from_uint64(*utils::floor_ptr<Elems>(ptr_));
            else {
                constexpr auto impl = []<std::size_t... Idx>(auto ptr, std::index_sequence<Idx...>) __aie_inline {
                    return inner_type::from_uint64((*(utils::floor_ptr<Elems>(ptr) + Idx))...);
                };
                v = impl(ptr_, std::make_index_sequence<Elems / 64u>{});
            }
            increment();
            return v;
        }
        else {
            inner_type v = inner_type(*this);
            increment();
            return v;
        }
    }

    __aie_inline
    constexpr void push(const vector_type& v) requires (innermost_level)
    {
        *ptr_ = v;
        increment();
    }

    __aie_inline
    void acquire()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().acquire();
            ptr_ = (native_type*)(port_.port().data());
        }
        return;
    }

    __aie_inline
    void release()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().release();
        }
        return;
    }

#if AIE_API_NATIVE
    using ptr_type = std::conditional_t<is_const, const native_type*, native_type*>;
#else
    using ptr_type = std::conditional_t<is_const,
                                        std::conditional_t<Restrict, const native_type* __restrict,
                                                                     const native_type*>,
                                        std::conditional_t<Restrict,       native_type* __restrict,
                                                                           native_type*>>;
#endif

    __aie_inline
    constexpr void increment(void)
    {
        const auto& inc = std::get<Level>(iter_desc_);
        if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_3d_byte(ptr_, inc.inc3,
                                       inc.num1, iter_state_.state_.c1, inc.inc1,
                                       inc.num2, iter_state_.state_.c2, inc.inc2);
        }
        else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_2d_byte(ptr_, inc.inc2,
                                       inc.num1, iter_state_.state_.c1, inc.inc1);
        }
        else if constexpr(std::is_same_v<contiguous_dim, std::decay_t<decltype(inc)>>) {
            constexpr unsigned incr = [](){
                if constexpr(std::is_same_v<native_type, vector<uint32, 4>>) return Elems / 128u;
                else if constexpr(std::is_same_v<native_type, mask64>)       return Elems / 64u;
                else                                                         return Elems / type_bits_v<native_type>;
            }();
            ptr_ += incr;
        }
        else {
            ptr_ = ::byte_incr(ptr_, inc);
        }
    }

    // Internal constructors for creating inner streams
    using outer_type = tbs_impl<T, Elems, Level == 0 ? 0 : Level-1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>;

    __aie_inline
    constexpr tbs_impl(const outer_type& other) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_) {}

    __aie_inline
    constexpr tbs_impl(const outer_type& other) requires (IOBuffer<U>) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_), port_(other.port_) {}


    ptr_type ptr_;
    iter_desc_storage iter_desc_;
    iter_state_storage iter_state_;
    io_buffer_storage<U> port_;
};

#if AIE_API_ML_VERSION >= 210
template <bool IsInnermost> struct internal_fifo_state {};

template <>
struct internal_fifo_state<true>
{
    unsigned load_count_;
    fifo_state_t state_;

    constexpr internal_fifo_state() : load_count_(0u) {
        state_.pos = 0;
    }
};
#endif

// block vector specializations
#if AIE_API_ML_VERSION >= 210

template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc,
          typename ResourceType, ResourceType Resource, tbs_mode Mode,
          bool Restrict,
          typename U>
    requires(!(is_stream<U>::value) && is_valid_block_type_v<T>)
struct tbs_impl<T, Elems, Level, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>
{
    using          elem_type = T;
    using        vector_type = tensor_desc_type_t<std::remove_const_t<elem_type>, Elems>;
    using         value_type = add_memory_bank_t<Resource, vector_type>;
    using            pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using     native_pointer = typename vector_type::native_pointer_type;
    using        iter_desc_t = std::decay_t<IterDesc&>;
    using  iter_desc_storage = iter_desc_t;
    using iter_state_storage = iter_state<std::tuple_element_t<Level, iter_desc_t>>;
    using          accum_tag = detail::accum_tag_for_type<elem_type, 32>;

    static constexpr bool next_sliding         = false;
    static constexpr bool innermost_level      = Level == NumLevels - 1;
    static constexpr bool innermost_contiguous = innermost_level && std::is_same_v<contiguous_dim, std::tuple_element_t<Level, iter_desc_t>>;
    static constexpr unsigned fill_freq        = block_vector_fill_frequency_v<elem_type>;
    using inner_type = std::conditional_t<innermost_level,
                                          vector_type,
                                          tbs_impl<T, Elems, Level + 1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>>;

    __aie_inline
    constexpr tbs_impl(U *ptr, const IterDesc& iter_desc) :
        ptr_((ptr_type)ptr), iter_desc_(iter_desc)
    {}

    __aie_inline
    constexpr tbs_impl(U &port, const IterDesc& iter_desc) requires (IOBuffer<U>) :
        ptr_((ptr_type)port.data()), iter_desc_(iter_desc), port_(port)
    {}

    __aie_inline
    constexpr inner_type pop()
    {
        if constexpr (innermost_level) {
            inner_type v;

            constexpr unsigned native_elems = native_block_vector_length_v<elem_type>;
            constexpr unsigned num_loads    = std::max(1u, Elems / native_elems);
            constexpr unsigned load_elems   = Elems / num_loads;

            if constexpr (innermost_contiguous) {
                utils::unroll_times<num_loads>([&](unsigned idx) __aie_inline {
                    if (!chess_manifest(fifo_.load_count_ != 0))
                        ::fifo_ld_fill(ptr_, fifo_.state_);

                    if constexpr (fill_freq == 0) fifo_.load_count_ = 1u;
                    else                          fifo_.load_count_ = (fifo_.load_count_ + 1u) % fill_freq;

                    v.template insert<load_elems>(idx, ::fifo_ld_pop(ptr_, fifo_.state_));
                });
            }
            else {
                if constexpr (num_loads == 1) {
                    ::fifo_ld_fill(ptr_, fifo_.state_);
                }
                else {
                    utils::unroll_times<num_loads - 1>([&](unsigned idx) __aie_inline {
                        if constexpr (fill_freq > 0) {
                            if (idx % fill_freq == 0)
                                ::fifo_ld_fill(ptr_, fifo_.state_);
                        }
                        else {
                            if (idx == 0)
                                ::fifo_ld_fill(ptr_, fifo_.state_);
                        }
                        v.template insert<load_elems>(idx, ::fifo_ld_pop(ptr_, fifo_.state_));
                    });
                }

                const auto& inc = std::get<Level>(iter_desc_);
                if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
                    v.template insert<load_elems>(num_loads-1, ::fifo_ld_pop_3d_byte(ptr_, fifo_.state_, inc.inc3,
                                                                                    inc.num1, iter_state_.state_.c1, inc.inc1,
                                                                                    inc.num2, iter_state_.state_.c2, inc.inc2));
                }
                else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
                    v.template insert<load_elems>(num_loads-1, ::fifo_ld_pop_2d_byte(ptr_, fifo_.state_, inc.inc2,
                                                                                     inc.num1, iter_state_.state_.c1, inc.inc1));
                }
                else {
                    v.template insert<load_elems>(num_loads-1, ::fifo_ld_pop_1d_byte(ptr_, fifo_.state_, inc));
                }
            }

            return v;
        }
        else {
            auto v = inner_type(*this);
            increment();
            return v;
        }
    }

    __aie_inline
    constexpr void push(const vector_type& v) requires (innermost_level)
    {
        constexpr unsigned native_elems = native_block_vector_length_v<elem_type>;
        constexpr unsigned num_stores   = std::max(1u, Elems / native_elems);
        constexpr unsigned store_elems = Elems / num_stores;

        utils::unroll_times<num_stores>([&](unsigned idx) __aie_inline {
            ::fifo_st_push(ptr_, v.template extract<store_elems>(idx), fifo_.state_);
        });

        if constexpr(!innermost_contiguous)
            ptr_ = st_flush(ptr_);
    }

    __aie_inline
    constexpr void push(const accum<accum_tag, Elems>& acc, int shift = 0) requires (innermost_level)
    {
        push(acc.template to_vector<elem_type>(shift));
    }

    __aie_inline
    void flush(void) requires (innermost_level)
    {
        ptr_ = st_flush(ptr_);
    }

    __aie_inline
    void acquire()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().acquire();
            ptr_ = (ptr_type) port_.port().data();
        }
        return;
    }

    __aie_inline
    void release()
    {
        if constexpr (AsyncIOBuffer<U>)
        {
            port_.port().release();
        }
        return;
    }

#if AIE_API_NATIVE
    using ptr_type = native_pointer*;
#else
    using ptr_type = std::conditional_t<Restrict,
                                        native_pointer* __restrict,
                                        native_pointer*>;
#endif

    __aie_inline
    constexpr auto st_flush(auto ptr)
    {
        const auto& inc = std::get<Level>(iter_desc_);
        if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
            ::fifo_st_flush_3d_byte(ptr, fifo_.state_, inc.inc3,
                    inc.num1, iter_state_.state_.c1, inc.inc1,
                    inc.num2, iter_state_.state_.c2, inc.inc2);
        }
        else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
            ::fifo_st_flush_2d_byte(ptr, fifo_.state_, inc.inc2,
                    inc.num1, iter_state_.state_.c1, inc.inc1);
        }
        else if constexpr(std::is_same_v<contiguous_dim, std::decay_t<decltype(inc)>>) {
            ::fifo_st_flush(ptr, fifo_.state_);
        }
        else {
            ::fifo_st_flush_1d_byte(ptr, fifo_.state_, inc);
        }
        return ptr;
    }

    __aie_inline
    constexpr void increment(void)
    {
        const auto& inc = std::get<Level>(iter_desc_);
        if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_3d_byte(ptr_, inc.inc3,
                                       inc.num1, iter_state_.state_.c1, inc.inc1,
                                       inc.num2, iter_state_.state_.c2, inc.inc2);
        }
        else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
            ptr_ =  ::add_2d_byte(ptr_, inc.inc2,
                                        inc.num1, iter_state_.state_.c1, inc.inc1);
        }
        else if constexpr(std::is_same_v<contiguous_dim, std::decay_t<decltype(inc)>>) {
            ptr_++;
        }
        else {
            ptr_ = ::byte_incr(ptr_, inc);
        }
    }

    // Internal constructors for creating inner streams
    using outer_type = tbs_impl<T, Elems, Level == 0 ? 0 : Level-1, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, false, U>;

    __aie_inline
    constexpr tbs_impl(const outer_type& other) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_)
    {}

    __aie_inline
    constexpr tbs_impl(const outer_type& other) requires (IOBuffer<U>) :
        ptr_(other.ptr_), iter_desc_(other.iter_desc_), port_(other.port_)
    {}

    ptr_type ptr_;
    iter_desc_storage iter_desc_;
    iter_state_storage iter_state_;
    internal_fifo_state<innermost_level> fifo_; // Only required on innermost_level
    io_buffer_storage<U> port_;
};

#endif //AIE_API_ML_VERSION >= 210

// general tbs interface class
template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc, data_layout Layout,
          typename ResourceType, ResourceType Resource, tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U>
struct tbs : private tbs_impl<T, Elems, Level, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, Unaligned, U>
{
    using base_type = tbs_impl<T, Elems, Level, NumLevels, SlidingInner, IterDesc, ResourceType, Resource, Mode, Restrict, Unaligned, U>;
    using elem_type = base_type::elem_type;
    using value_type = base_type::value_type;

    using base_type::base_type;
    using base_type::acquire;
    using base_type::release;
    using inner_type = typename base_type::inner_type;

    static constexpr data_layout layout = Layout;

    __aie_inline
    auto pop()
    {
        if constexpr (base_type::innermost_level || base_type::next_sliding) {
            return base_type::pop();
        }
        else {
            using next_type = tbs<T, Elems, Level + 1, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>;
            return next_type{base_type::pop()};
        }
    }

    __aie_inline
    void push(const auto& v) requires (base_type::innermost_level &&
                                       !std::is_same_v<ResourceType, aie_stream_resource_in>)
    {
        base_type::push(v);
    }

    // Accept accum types and optional parameters like shift and is_last and forward them to the underlying base type implementation
    template <typename... Args>
    __aie_inline
    void push(Args&&... args) requires (base_type::innermost_level &&
                                        !std::is_same_v<ResourceType, aie_stream_resource_in> &&
                                        requires(base_type& b) { b.push(std::forward<Args>(args)...); })
    {
        base_type::push(std::forward<Args>(args)...);
    }

    __aie_inline
    void flush(void) requires (base_type::innermost_level)
    {
        // Flushing is only required for, and therefore implemented on, fifo-based interfaces
        if constexpr (requires { base_type::flush(); })
            base_type::flush();
    }

    __aie_inline
    tbs &operator>>(inner_type& v) requires (base_type::innermost_level)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    tbs &operator<<(const inner_type& v) requires (base_type::innermost_level)
    {
        push(v);
        return *this;
    }

private:
    friend struct tbs<T, Elems, Level - 1, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>;

    explicit tbs(const base_type &rhs) : base_type(rhs) {}
};
#endif

template <typename ResourceType,
          ResourceType Resource,
          tbs_mode Mode,
          bool Restrict = false, bool Unaligned = false,
          typename T,
          typename Desc>
auto make_tensor_buffer_stream(T&& src, const Desc& desc)
{
    using                    elem_type = typename Desc::type;
    constexpr unsigned           elems = Desc::elems;
    constexpr unsigned      init_level = 0;
    constexpr unsigned      num_levels = Desc::num_levels;
    constexpr bool     has_sliding_dim = Desc::has_innermost_sliding_dim;
    constexpr data_layout       layout = Desc::layout;

    using                  iter_desc_t = typename Desc::tensor_iteration_descriptor;

    static_assert(!has_sliding_dim || std::is_same_v<ResourceType, aie_dm_resource>,
                  "Sliding windows are only compatible with memory reads");

    // T = int8*, const int8*, input_stream<int8>*, input_buffer<int8>&
    // U = int8,  const int8,  input_stream<int8>,  input_buffer<int8>
    using U = std::remove_pointer_t<std::remove_reference_t<T>>;

    if constexpr (num_levels == 1 && has_sliding_dim) {
        return const_sliding_window_buffer_stream<elem_type, elems,
                                                  std::tuple_element_t<0, iter_desc_t>,
                                                  Resource>(std::forward<T&&>(src), std::get<0>(desc.it_desc_));
    }
    else {
        return tbs<elem_type, elems,
                   init_level, num_levels, has_sliding_dim, iter_desc_t, layout,
                   ResourceType, Resource, Mode, Restrict, Unaligned, U>(std::forward<T&&>(src), desc.it_desc_);
    }
}

template <size_t Stride>
struct iterator_stride
{
    static constexpr size_t value() { return Stride; }
};

template <>
struct iterator_stride<dynamic_extent>
{
    size_t stride_;

    constexpr size_t value() const { return stride_; }
};

enum class FifoDirection {
    In,
    Out
};

template <typename Pointer, size_t Elems, size_t Stride>
struct random_circular_iterator_storage;

} // namespace aie::detail

#if __AIE_ARCH__ == 10

#include "aie1/array_helpers.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/array_helpers.hpp"

#elif __AIE_ARCH__ == 21

#include "aie2/array_helpers.hpp"
#include "aie2p/array_helpers.hpp"

#elif __AIE_ARCH__ == 22

#include "aie2/array_helpers.hpp"
#include "aie2p/array_helpers.hpp"

#endif

namespace aie::detail {

template <typename Pointer, size_t Elems>
struct __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator_storage_static
{
    Pointer ptr;
    Pointer base;
    static constexpr size_t elems = Elems;
};

template <typename Pointer>
struct __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator_storage_dynamic
{
    Pointer ptr;
    Pointer base;
    size_t elems;
};

template <typename T, size_t Elems, size_t Stride, aie_dm_resource Resource>
class __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator
{
    static constexpr bool is_static()
    {
        return Elems != dynamic_extent;
    }

    static constexpr bool is_stride_static()
    {
        return Stride != dynamic_extent;
    }

public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type* ;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t elems, size_t stride) :
        storage_{ptr, ptr, elems},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t elems) :
        storage_{ptr, ptr, elems}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t stride) :
        storage_{ptr, ptr},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr) :
        storage_{ptr, ptr}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t elems, size_t stride) :
        storage_{ptr, base, elems},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t elems) :
        storage_{ptr, base, elems}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t stride) :
        storage_{ptr, base},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
        constexpr random_circular_iterator(pointer ptr, pointer base) :
        storage_{ptr, base}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    random_circular_iterator  &operator++()
    {
        *this += 1;
        return *this;
    }

    random_circular_iterator   operator++(int)
    {
        random_circular_iterator it = *this;
        ++(*this);
        return it;
    }

    random_circular_iterator  &operator--()
    {
        *this -= 1;
        return *this;
    }

    random_circular_iterator   operator--(int)
    {
        random_circular_iterator it = *this;
        --(*this);
        return it;
    }

    random_circular_iterator   operator+(int off) const
    {
        random_circular_iterator it = *this;
        it += off;
        return it;
    }

    random_circular_iterator   operator-(int off) const
    {
        random_circular_iterator it = *this;
        it -= off;
        return it;
    }

    random_circular_iterator  &operator+=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,  off * stride_.value(), storage_.base, storage_.elems);
        return *this;
    }

    random_circular_iterator  &operator-=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr, -off * stride_.value(), storage_.base, storage_.elems);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()                                           { return *storage_.ptr;                    }
    constexpr pointer   operator->()                                          { return storage_.ptr;                     }
    constexpr bool      operator==(const random_circular_iterator& rhs) const { return storage_.ptr == rhs.storage_.ptr; }
    constexpr bool      operator!=(const random_circular_iterator& rhs) const { return storage_.ptr != rhs.storage_.ptr; }

private:
    using storage_type = std::conditional_t<is_static(),
                                            random_circular_iterator_storage_static<pointer, Elems>,
                                            random_circular_iterator_storage_dynamic<pointer>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Elems, size_t ArrayElems, size_t Stride, aie_dm_resource Resource>
class vector_random_circular_iterator
{
    static constexpr bool is_static()
    {
        return ArrayElems != dynamic_extent;
    }

    static constexpr bool is_stride_static()
    {
        return Stride != dynamic_extent;
    }

public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<elem_type, Elems>, aie_dm_resource_get_v<T>>>;

    static constexpr unsigned subbyte_elems = type_bits_v<elem_type> == 4? 2 : 1;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t elems, size_t stride) :
        storage_{ptr, ptr, elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t elems) :
        storage_{ptr, ptr, elems}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t stride) :
        storage_{ptr, ptr},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr) :
        storage_{ptr, ptr}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t elems, size_t stride) :
        storage_{ptr, base, elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t elems) :
        storage_{ptr, base, elems}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t stride) :
        storage_{ptr, base},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                     "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,     "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
        constexpr vector_random_circular_iterator(T *ptr, T *base) :
        storage_{ptr, base}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                     "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,     "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    vector_random_circular_iterator  &operator++()
    {
        *this += 1;
        return *this;
    }

    vector_random_circular_iterator   operator++(int)
    {
        vector_random_circular_iterator it = *this;
        ++(*this);
        return it;
    }

    vector_random_circular_iterator  &operator--()
    {
        *this -= 1;
        return *this;
    }

    vector_random_circular_iterator   operator--(int)
    {
        vector_random_circular_iterator it = *this;
        --(*this);
        return it;
    }

    vector_random_circular_iterator   operator+(int off) const
    {
        vector_random_circular_iterator it = *this;
        it += off;
        return it;
    }

    vector_random_circular_iterator   operator-(int off) const
    {
        vector_random_circular_iterator it = *this;
        it -= off;
        return it;
    }

    vector_random_circular_iterator  &operator+=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,
                                    (off * (Elems / subbyte_elems) * stride_.value()),
                                    storage_.base,
                                    storage_.elems / subbyte_elems);

        return *this;
    }

    vector_random_circular_iterator  &operator-=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,
                                    -int(off * (Elems / subbyte_elems) * stride_.value()),
                                    storage_.base,
                                    storage_.elems / subbyte_elems);

        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(pointer)*this + off / subbyte_elems;
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(pointer)*this + off / subbyte_elems;
    }

    constexpr reference operator*()                                                  { return *(pointer)storage_.ptr;           }
    constexpr pointer   operator->()                                                 { return (pointer)storage_.ptr;            }
    constexpr bool      operator==(const vector_random_circular_iterator& rhs) const { return storage_.ptr == rhs.storage_.ptr; }
    constexpr bool      operator!=(const vector_random_circular_iterator& rhs) const { return storage_.ptr != rhs.storage_.ptr; }

private:
    using storage_type = std::conditional_t<is_static(),
                                            random_circular_iterator_storage_static<T *, ArrayElems>,
                                            random_circular_iterator_storage_dynamic<T *>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Elems, size_t Stride, aie_dm_resource Resource = aie_dm_resource::none>
class __AIE_API_KEEP_IN_REGISTERS__ vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <size_t Stride2 = Stride> requires(Stride2 == dynamic_extent)
    constexpr vector_iterator(T *ptr, size_t stride = 1) :
        ptr_(ptr),
        stride_{stride}
    {
    }

    template <size_t Stride2 = Stride> requires(Stride2 != dynamic_extent)
    constexpr vector_iterator(T *ptr) :
        ptr_(ptr)
    {
    }

    constexpr vector_iterator &operator++()
    {
        *this += 1;
        return *this;
    }

    constexpr vector_iterator  operator++(int)
    {
        const vector_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr vector_iterator &operator--()
    {
        *this -= 1;
        return *this;
    }

    constexpr vector_iterator  operator--(int)
    {
        const vector_iterator it = *this;
        --(*this);
        return it;
    }

    constexpr vector_iterator operator+(difference_type off) const
    {
        vector_iterator it = *this;
        it += off;
        return it;
    }

    constexpr vector_iterator operator-(difference_type off) const
    {
        vector_iterator it = *this;
        it -= off;
        return it;
    }

    constexpr vector_iterator &operator+=(difference_type off)
    {
        ptr_ += increment(off);
        return *this;
    }

    constexpr vector_iterator &operator-=(difference_type off)
    {
        ptr_ -= increment(off);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()
    {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");
#endif

        return *(pointer)ptr_;
    }

    constexpr pointer   operator->()
    {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");
#endif

        return (pointer)ptr_;
    }

    constexpr bool      operator==(const vector_iterator& rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool      operator!=(const vector_iterator& rhs) const { return ptr_ != rhs.ptr_; }

private:

    constexpr size_t increment(difference_type off) const
    {
        if constexpr (type_bits_v<std::remove_cv_t<T>> == 4)
            return (Elems / (sizeof(T) * 2)) * off * stride_.value();
        else
            return Elems * off * stride_.value();
    }

    T *ptr_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

//TODO : Improve / refactor this code (CRVO-3256)
template <typename T, unsigned Elems, size_t Stride, aie_dm_resource Resource = aie_dm_resource::none>
class __AIE_API_KEEP_IN_REGISTERS__ restrict_vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <size_t Stride2 = Stride> requires(Stride2 == dynamic_extent)
    constexpr restrict_vector_iterator(T * __restrict ptr, size_t stride = 1) :
        ptr_(ptr),
        stride_{stride}
    {
    }

    template <size_t Stride2 = Stride> requires(Stride2 != dynamic_extent)
    constexpr restrict_vector_iterator(T * __restrict ptr) :
        ptr_(ptr)
    {
    }

    constexpr restrict_vector_iterator &operator++()
    {
        *this += 1;
        return *this;
    }

    constexpr restrict_vector_iterator  operator++(int)
    {
        const restrict_vector_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr restrict_vector_iterator &operator--()
    {
        *this -= 1;
        return *this;
    }

    constexpr restrict_vector_iterator  operator--(int)
    {
        const restrict_vector_iterator it = *this;
        --(*this);
        return it;
    }

    constexpr restrict_vector_iterator operator+(difference_type off) const
    {
        restrict_vector_iterator it = *this;
        it += off;
        return it;
    }

    constexpr restrict_vector_iterator operator-(difference_type off) const
    {
        restrict_vector_iterator it = *this;
        it -= off;
        return it;
    }

    constexpr restrict_vector_iterator &operator+=(difference_type off)
    {
        ptr_ += increment(off);
        return *this;
    }

    constexpr restrict_vector_iterator &operator-=(difference_type off)
    {
        ptr_ -= increment(off);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()
    {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");
#endif

        pointer __restrict tmp_ptr = (pointer) ptr_;
        return *tmp_ptr;
    }

    constexpr pointer operator->()
    {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");
#endif

        pointer __restrict tmp_ptr = (pointer) ptr_;
        return tmp_ptr;
    }

    constexpr bool      operator==(const restrict_vector_iterator& rhs) const { return ptr_ == rhs.ptr_;     }
    constexpr bool      operator!=(const restrict_vector_iterator& rhs) const { return ptr_ != rhs.ptr_;     }

private:

    constexpr size_t increment(difference_type count) const
    {
        if constexpr (type_bits_v<std::remove_cv_t<T>> == 4)
            return (Elems / (sizeof(T) * 2)) * count * stride_.value();
        else
            return Elems * count * stride_.value();
    }

    T * __restrict ptr_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Steps>
class pattern_iterator
{
    // TODO: check if using property(keep_in_registers) helps

public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <typename... StepOffsets>
    constexpr pattern_iterator(pointer ptr, StepOffsets... step_offsets) :
        ptr_(ptr),
        offsets_{step_offsets...},
        idx_(offsets_)
    {
        static_assert(sizeof...(StepOffsets) == Steps);
    }

    constexpr pattern_iterator &operator++()
    {
        ptr_ += *(idx_++);
        return *this;
    }

    constexpr pattern_iterator  operator++(int)
    {
        const pattern_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr reference operator*()                                   { return *ptr_;            }
    constexpr pointer   operator->()                                  { return ptr_;             }
    constexpr bool      operator==(const pattern_iterator& rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool      operator!=(const pattern_iterator& rhs) const { return ptr_ != rhs.ptr_; }

private:

    T *ptr_;
    ptrdiff_t offsets_[Steps];
    circular_iterator<ptrdiff_t, Steps, 1, aie_dm_resource::none> idx_;
};

// TODO: Add optimized implementations for AIE1/AIE2
template <typename T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<std::remove_const_t<elem_type>, Elems>;
    using   vector_ref_type = unaligned_vector_ref<T,
                                                   vector_type::size(),
                                                   Resource>;

    using        value_type = vector_type;
    using         reference = vector_ref_type;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    __aie_inline
    constexpr unaligned_vector_iterator(T *ptr) :
        ptr_(ptr)
    {
        RUNTIME_ASSERT(ptr_ != nullptr, "Iterator cannot be created from nullptr");

        if constexpr (type_bits_v<T> >= 8)
            alignment_ = 1 << utils::ffs(uintptr_t(ptr) / (type_bits_v<T> / 8));
        else if constexpr (type_bits_v<T> == 4)
            alignment_ = 2 << utils::ffs(uintptr_t(ptr));
    }

    __aie_inline
    constexpr unaligned_vector_iterator &operator++()
    {
        ptr_ += Elems;
        return *this;
    }

    __aie_inline
    constexpr unaligned_vector_iterator  operator++(int)
    {
        unaligned_vector_iterator it = *this;
        ++(*this);
        return it;
    }

    __aie_inline
    constexpr vector_ref_type operator*() const
    {
        return vector_ref_type(ptr_, alignment_);
    }

    __aie_inline
    constexpr vector_ref_type operator*() requires(!std::is_const_v<T>)
    {
        return vector_ref_type(ptr_, alignment_);
    }

    constexpr bool operator==(const unaligned_vector_iterator &rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool operator!=(const unaligned_vector_iterator &rhs) const { return ptr_ != rhs.ptr_; }

private:

    T *ptr_;
    unsigned alignment_;
};

// Default implementation of input and output buffer streams, relying on vector_iterator and unaligned_vector_iterator.
// The architecture backends can provide optimized specializations.
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class vector_input_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit vector_input_buffer_stream(const T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    constexpr vector_type pop()
    {
        vector_type v = *it_; ++it_;
        return v;
    }

private:
    vector_iterator<const T, Elems, 1, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class vector_output_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit vector_output_buffer_stream(T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr vector_output_buffer_stream &operator<<(const vector_type &v)
    {
        push(v);
        return *this;
    }

    __aie_inline
    constexpr void push(const vector_type &v)
    {
        *it_ = v; ++it_;
    }

private:
    vector_iterator<T, Elems, 1, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_input_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit unaligned_vector_input_buffer_stream(const T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr unaligned_vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    constexpr vector_type pop()
    {
        vector_type v = *it_; ++it_;
        return v;
    }

private:

    unaligned_vector_iterator<const T, Elems, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_output_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit unaligned_vector_output_buffer_stream(T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr unaligned_vector_output_buffer_stream &operator<<(const vector_type &v)
    {
        push(v);
        return *this;
    }

    __aie_inline
    constexpr void push(const vector_type &v)
    {
        *it_ = v; ++it_;
    }

private:

    unaligned_vector_iterator<T, Elems, Resource> it_;
};

// Operator overloading to allow output << input patterns to avoid having to store vectors in temporary variables
template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr vector_output_buffer_stream<T, Elems, Resource> &
operator<<(vector_output_buffer_stream<T, Elems, Resource> &out, vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr unaligned_vector_output_buffer_stream<T, Elems, Resource> &
operator<<(unaligned_vector_output_buffer_stream<T, Elems, Resource> &out, vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr vector_output_buffer_stream<T, Elems, Resource> &
operator<<(vector_output_buffer_stream<T, Elems, Resource> &out, unaligned_vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename unaligned_vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr unaligned_vector_output_buffer_stream<T, Elems, Resource> &
operator<<(unaligned_vector_output_buffer_stream<T, Elems, Resource> &out, unaligned_vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename unaligned_vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

#if AIE_API_ML_VERSION >= 200

template <DecoratedElemBaseType T, unsigned N, aie_dm_resource Resource>
class __AIE_API_KEEP_IN_REGISTERS__ sparse_vector_input_buffer_stream :
    public fifo_buffer_stream<sparse_vector<std::remove_const_t<aie_dm_resource_remove_t<T>>, N>, Resource, FifoDirection::In>
{
public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = sparse_vector<elem_type, N>;

    using        value_type = vector_type;

    __aie_inline
    constexpr sparse_vector_input_buffer_stream(const T *ptr) :
        parent_type(ptr)
    {
    }

    __aie_inline
    constexpr sparse_vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = this->pop();
        return *this;
    }

private:
    using parent_type = fifo_buffer_stream<vector_type, Resource, FifoDirection::In>;
};

#endif

#if AIE_API_ML_VERSION >= 210

template <BlockType T, unsigned N, aie_dm_resource Resource, bool Restrict>
class __AIE_API_KEEP_IN_REGISTERS__ block_vector_input_buffer_stream :
    public fifo_buffer_stream<block_vector<std::remove_const_t<aie_dm_resource_remove_t<T>>, N>, Resource, FifoDirection::In, Restrict>

{
public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = block_vector<elem_type, N>;

    using        value_type = vector_type;

    constexpr block_vector_input_buffer_stream(const T *ptr) :
        parent_type(ptr)
    {
    }

    __aie_inline
    constexpr block_vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = this->pop();
        return *this;
    }

private:
    using parent_type = fifo_buffer_stream<vector_type, Resource, FifoDirection::In, Restrict>;
};

template <BlockType T, unsigned N, aie_dm_resource Resource, bool Restrict>
class __AIE_API_KEEP_IN_REGISTERS__ block_vector_output_buffer_stream :
    public fifo_buffer_stream<block_vector<std::remove_const_t<aie_dm_resource_remove_t<T>>, N>, Resource, FifoDirection::Out, Restrict>
{
public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = block_vector<elem_type, N>;
    using        accum_type = accum<accum_tag_for_type<elem_type>, N>;

    using        value_type = vector_type;

    __aie_inline
    constexpr block_vector_output_buffer_stream(T *ptr) :
        parent_type(ptr)
    {
    }

    __aie_inline
    constexpr block_vector_output_buffer_stream &operator<<(const vector_type &v)
    {
        this->push(v);
        return *this;
    }

    __aie_inline
    constexpr block_vector_output_buffer_stream &operator<<(const accum_type &acc)
    {
        this->push(acc.template to_vector<elem_type>());
        return *this;
    }

private:
    using parent_type = fifo_buffer_stream<vector_type, Resource, FifoDirection::Out, Restrict>;
};
#endif

} // namespace aie::detail

#endif
