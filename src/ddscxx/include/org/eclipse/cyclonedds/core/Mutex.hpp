/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */


/**
 * @file
 */

#ifndef CYCLONEDDS_CORE_MUTEX_HPP_
#define CYCLONEDDS_CORE_MUTEX_HPP_

#include <dds/core/macros.hpp>

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace core
{

/**
  * @brief Mutex wrapper class.
  *
  * This class wraps the CycloneDDS-C mutex implementation.
  */
class OMG_DDS_API Mutex
{
public:
    /**
      * @brief Constructor.
      *
      * Creates and initializes the CycloneDDS-C mutex (ddsrt_mutex_t).
      */
    Mutex();
    /**
      * @brief Destructor (virtual).
      *
      * Cleans up resources associated with the CycloneDDS-C mutex.
      */
    virtual ~Mutex();

    /**
      * @brief Locking function.
      *
      * Will block the calling thread until the lock can be acquired.
      */
    void lock() const;
    /**
      * @brief Attempt locking function.
      *
      * Will attempt to acquire the lock, and returns whether this was succesful.
      *
      * @return bool whether the lock was acquired.
      */
    bool try_lock() const;
    /**
      * @brief Unlocking function.
      *
      * Will unlock the mutex.
      */
    void unlock() const;
private:
    void* mtx;
};

}
}
}
}


#endif /* CYCLONEDDS_CORE_MUTEX_HPP_ */
