// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_LOCK__HPP__
#define __AIE_API_DETAIL_LOCK__HPP__

#include "utils.hpp"

#include <cstddef>
#include <tuple>
#include <utility>

namespace aie::detail {

struct adopt_lock_t {};

// Forward declaration for classes implemented in the architecture backends
class          mutex;
template <unsigned NumReaders, unsigned NumWriters>
class producer_sem;
template <unsigned NumReaders, unsigned NumWriters>
class consumer_sem;

}

#if __AIE_ARCH__ == 10

#include "aie1/lock.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/lock.hpp"

#endif

namespace aie::detail {

template <typename MutexType>
class scoped_lock_impl {
    using  mutex_type = MutexType;

    mutex_type *m_;

private:
    // Not copy-constructible and not copy-assignable
    scoped_lock_impl(const scoped_lock_impl &)            = delete;
    scoped_lock_impl &operator=(const scoped_lock_impl &) = delete;

    void lock()
    {
        m_->lock();
    }

    void unlock()
    {
        if (m_)
            m_->unlock();
    }

public:
    explicit scoped_lock_impl(mutex_type &m) :
        m_(&m)
    {
        lock();
    }

    explicit scoped_lock_impl(adopt_lock_t, mutex_type &m) :
        m_(&m)
    {
    }

    ~scoped_lock_impl()
    {
        unlock();
    }

    scoped_lock_impl(scoped_lock_impl && l)
    {
        unlock();

        m_    = l->m_;
        l->m_ = nullptr;
    }

    scoped_lock_impl &operator=(scoped_lock_impl && l)
    {
        unlock(*m_);

        m_    = l->m_;
        l->m_ = nullptr;

        return *this;
    }
};

template <size_t... Indices, typename... MutexTypes>
void unlock_reverse_helper(const std::index_sequence<Indices...> &, MutexTypes && ...mutexes)
{
    const auto t = std::forward_as_tuple(std::forward<MutexTypes>(mutexes)...);

    (std::get<Indices>(t).unlock(), ...);
}

template <typename Fn, typename... MutexTypes>
void locked(Fn fn, MutexTypes & ...mutexes)
{
    (mutexes.lock(), ...);

    fn();

    unlock_reverse_helper(utils::make_reverse_index_sequence<sizeof...(MutexTypes)>{}, std::forward<MutexTypes>(mutexes)...);
}

using   scoped_lock = scoped_lock_impl<mutex>;

template <unsigned NumReaders, unsigned NumWriters>
using producer_lock = scoped_lock_impl<producer_sem<NumReaders, NumWriters>>;
template <unsigned NumReaders, unsigned NumWriters>
using consumer_lock = scoped_lock_impl<consumer_sem<NumReaders, NumWriters>>;

}

#endif
