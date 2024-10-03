// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_LOCK__HPP__
#define __AIE_API_DETAIL_AIE1_LOCK__HPP__

namespace aie::detail {

class mutex {
private:
    const unsigned lock_id_;

    mutex()                         = delete;
    mutex(const mutex &)            = delete;
    mutex(mutex &&)                 = delete;

    mutex &operator=(const mutex &) = delete;
    mutex &operator=(mutex &&)      = delete;

public:
    void lock()
    {
        ::acquire(lock_id_);
    }

    void unlock()
    {
        ::release(lock_id_);
    }

    mutex(unsigned lock_id) :
        lock_id_(lock_id)
    {
    }
};

template <unsigned Readers, unsigned Writers>
class producer_sem {
private:
    const unsigned lock_id_;

    static_assert(Readers == 1);
    static_assert(Writers == 1);

    producer_sem()                                = delete;
    producer_sem(const producer_sem &)            = delete;
    producer_sem(producer_sem &&)                 = delete;

    producer_sem &operator=(const producer_sem &) = delete;
    producer_sem &operator=(producer_sem &&)      = delete;

public:
    void lock()
    {
        ::acquire(lock_id_, 0);
    }

    void unlock()
    {
        ::release(lock_id_, 1);
    }

    producer_sem(unsigned lock_id) :
        lock_id_(lock_id)
    {
    }
};

template <unsigned Readers, unsigned Writers>
class consumer_sem {
private:
    const unsigned lock_id_;

    static_assert(Readers == 1);
    static_assert(Writers == 1);

    consumer_sem()                                = delete;
    consumer_sem(const consumer_sem &)            = delete;
    consumer_sem(consumer_sem &&)                 = delete;

    consumer_sem &operator=(const consumer_sem &) = delete;
    consumer_sem &operator=(consumer_sem &&)      = delete;

public:
    void lock()
    {
        ::acquire(lock_id_, 1);
    }

    void unlock()
    {
        ::release(lock_id_, 0);
    }

    consumer_sem(unsigned lock_id) :
        lock_id_(lock_id)
    {
    }
};

}

#endif
