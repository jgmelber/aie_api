// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SYNC_BUFFER__HPP__
#define __AIE_API_DETAIL_AIE2_SYNC_BUFFER__HPP__

#include "../lock.hpp"
#include "../vector.hpp"
#include "../mdspan.hpp"

namespace aie::detail::sync {

template <unsigned NumBuffers>
class sync_data_index
{
    addr_t idx_ = 0;

public:
    constexpr unsigned get_index() const
    {
        return (unsigned)idx_;
    }

    constexpr unsigned operator++()
    {
        if constexpr (NumBuffers == 2)
            idx_ = 1 - idx_;
        else if constexpr (NumBuffers > 2)
            ::add_2d_ptr((char *)uintptr_t(0), -int(NumBuffers - 1), NumBuffers - 1, idx_, 1);

        return idx_;
    }
};

template <>
class sync_data_index<1>
{
public:
    constexpr unsigned get_index() const
    {
        return 0;
    }

    constexpr unsigned operator++()
    {
        return 0;
    }
};

template <direction Direction, typename Span, unsigned NumBuffers, unsigned NumReaders, unsigned NumWriters, size_t... Is>
class sync_data_impl<Direction, Span, NumBuffers, NumReaders, NumWriters, std::index_sequence<Is...>>
{
    static_assert(NumBuffers > 0);
    static_assert(NumReaders > 0);
    static_assert(NumWriters > 0);

    using   span_type = Span;
    using           T = typename span_type::value_type;
    using  mutex_type = std::conditional_t<(Direction == direction::Input), aie::detail::consumer_sem<NumReaders, NumWriters>,
                                                                            aie::detail::producer_sem<NumReaders, NumWriters>>;

    template <size_t>
    using   buffer_ptr = T *;

public:
    using value_type = T;

    static constexpr unsigned num_buffers = NumBuffers;
    static constexpr unsigned num_readers = NumReaders;
    static constexpr unsigned num_writers = NumWriters;

    template <typename... DynamicExtents>
    constexpr sync_data_impl(buffer_ptr<Is>  ...ptrs,
                             mutex_type        &mutexes,
                             DynamicExtents  ...ext) :
        buffers_{Span(ptrs, ext...)...},
        locks_{mutexes}
    {
        static_assert(sizeof...(ptrs) == num_buffers);
        static_assert(sizeof...(ext)  == Span::rank_dynamic());
    }

    Span &acquire()
    {
        this->locks_.lock();

        return this->buffers_[index_.get_index()];
    }

    void release()
    {
        this->locks_.unlock();

        ++index_;
    }

    constexpr unsigned current_index() const
    {
        return index_.get_index();
    }

    constexpr size_t size() const
    {
        return buffers_[0].size();
    }

    constexpr size_t bytes() const
    {
        return size() * type_bits_v<T>;
    }

private:
    Span buffers_[NumBuffers];
    mutex_type &locks_;
    [[no_unique_address]] sync_data_index<NumBuffers> index_;
};

template <direction Direction, typename T, unsigned NumBuffers, unsigned NumReaders, unsigned NumWriters, size_t... Is>
class sync_data_impl<Direction, T *, NumBuffers, NumReaders, NumWriters, std::index_sequence<Is...>>
{
    static_assert(NumBuffers > 0);
    static_assert(NumReaders > 0);
    static_assert(NumWriters > 0);

    using mutex_type = std::conditional_t<(Direction == direction::Input), aie::detail::consumer_sem<NumReaders, NumWriters>,
                                                                           aie::detail::producer_sem<NumReaders, NumWriters>>;

    template <size_t>
    using buffer_ptr = T *;

public:
    using value_type = T;

    static constexpr unsigned num_buffers = NumBuffers;
    static constexpr unsigned num_readers = NumReaders;
    static constexpr unsigned num_writers = NumWriters;

    constexpr sync_data_impl(buffer_ptr<Is> ...ptrs,
                             mutex_type       &mutexes,
                             size_t            size) :
        buffers_{ptrs...},
        locks_{mutexes},
        size_(size)
    {
        static_assert(sizeof...(ptrs) == num_buffers);
    }

    value_type *acquire()
    {
        this->locks_.lock();

        return this->buffers_[index_.get_index()];
    }

    void release()
    {
        this->locks_.unlock();

        ++index_;
    }

    constexpr unsigned current_index() const
    {
        return index_.get_index();
    }

    constexpr size_t size() const
    {
        return size_;
    }

    constexpr size_t bytes() const
    {
        return size() * type_bits_v<T>;
    }

private:
    T *buffers_[NumBuffers];
    mutex_type &locks_;
    size_t size_;
    [[no_unique_address]] sync_data_index<NumBuffers> index_;
};

template <direction Direction, typename T, size_t Elems, unsigned NumBuffers, unsigned NumReaders, unsigned NumWriters, size_t... Is>
class sync_data_impl<Direction, T[Elems], NumBuffers, NumReaders, NumWriters, std::index_sequence<Is...>>
{
    static_assert(NumBuffers > 0);
    static_assert(NumReaders > 0);
    static_assert(NumWriters > 0);

    using mutex_type = std::conditional_t<(Direction == direction::Input), aie::detail::consumer_sem<NumReaders, NumWriters>,
                                                                           aie::detail::producer_sem<NumReaders, NumWriters>>;

    template <size_t>
    using buffer_ptr = T *;

public:
    using value_type = T;

    static constexpr unsigned num_buffers = NumBuffers;
    static constexpr unsigned num_readers = NumReaders;
    static constexpr unsigned num_writers = NumWriters;

    constexpr sync_data_impl(buffer_ptr<Is> ...ptrs,
                             mutex_type       &mutexes) :
        buffers_{ptrs...},
        locks_{mutexes}
    {
        static_assert(sizeof...(ptrs) == num_buffers);
    }

    value_type *acquire()
    {
        this->locks_.lock();

        return this->buffers_[index_.get_index()];
    }

    void release()
    {
        this->locks_.unlock();

        ++index_;
    }

    constexpr unsigned current_index() const
    {
        return index_.get_index();
    }

    constexpr size_t size() const
    {
        return Elems;
    }

    constexpr size_t bytes() const
    {
        return size() * type_bits_v<T>;
    }

private:
    T *buffers_[NumBuffers];
    mutex_type &locks_;
    [[no_unique_address]] sync_data_index<NumBuffers> index_;
};

}

#endif
