// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_LOCK__HPP__
#define __AIE_API_DETAIL_AIE2_LOCK__HPP__

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
        ::acquire_greater_equal(lock_id_, 1);
    }

    void unlock()
    {
        ::release(lock_id_, 1);
    }

    mutex(unsigned lock_id) :
        lock_id_(lock_id)
    {
        ::release(lock_id_, 1);
    }
};

template <unsigned Readers, unsigned Writers>
class producer_sem;

template <>
class producer_sem<1, 1>
{
private:
    const unsigned lock_read_id_;
    const unsigned lock_write_id_;

    producer_sem()                                = delete;
    producer_sem(const producer_sem &)            = delete;
    producer_sem(producer_sem &&)                 = delete;

    producer_sem &operator=(const producer_sem &) = delete;
    producer_sem &operator=(producer_sem &&)      = delete;

public:
    void lock()
    {
        ::acquire_greater_equal(lock_write_id_, 1);
    }

    void unlock()
    {
        ::release(lock_read_id_,  1);
    }

    producer_sem(unsigned lock_read_id, unsigned lock_write_id) :
        lock_read_id_(lock_read_id),
        lock_write_id_(lock_write_id)
    {
    }
};

template <unsigned Readers, unsigned Writers>
class consumer_sem;

template <>
class consumer_sem<1, 1> {
private:
    const unsigned lock_read_id_;
    const unsigned lock_write_id_;

    consumer_sem()                                = delete;
    consumer_sem(const consumer_sem &)            = delete;
    consumer_sem(consumer_sem &&)                 = delete;

    consumer_sem &operator=(const consumer_sem &) = delete;
    consumer_sem &operator=(consumer_sem &&)      = delete;

public:
    void lock()
    {
        ::acquire_greater_equal(lock_read_id_,  1);
    }

    void unlock()
    {
        ::release(lock_write_id_, 1);
    }

    consumer_sem(unsigned lock_read_id, unsigned lock_write_id) :
        lock_read_id_(lock_read_id),
        lock_write_id_(lock_write_id)
    {
    }
};

}

#endif
