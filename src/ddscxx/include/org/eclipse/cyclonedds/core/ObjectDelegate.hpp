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

#ifndef CYCLONEDDS_CORE_OBJECT_DELEGATE_HPP_
#define CYCLONEDDS_CORE_OBJECT_DELEGATE_HPP_

#include "dds/core/macros.hpp"
#include "dds/core/refmacros.hpp"
#include "org/eclipse/cyclonedds/core/Mutex.hpp"

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace core
{

DDSCXX_WARNING_MSVC_OFF(4251)

/**
  * @brief CycloneDDS object helper class.
  *
  * Responsible for handling refcounts of the object and locks on the object.
  */
class OMG_DDS_API ObjectDelegate
{
public:
    /**
      * @brief Convenience typedef.
      */
    typedef ::dds::core::smart_ptr_traits< ObjectDelegate >::ref_type ref_type;
    /**
      * @brief Convenience typedef.
      */
    typedef ::dds::core::smart_ptr_traits< ObjectDelegate >::weak_ref_type weak_ref_type;

    /**
      * @brief Default constructor.
      */
    ObjectDelegate ();
    /**
      * @brief Destructor (virtual).
      */
    virtual ~ObjectDelegate ();

    /**
      * @brief Close function.
      *
      * This function indicates to DDS that the object will no longer be valid, and
      * no longer participate.
      */
    virtual void close ();

    /**
      * @brief Lock function.
      *
      * Calling this function will block execution on the calling thread until the mutex can be acquired.
      */
    void lock() const;

    /**
      * @brief Unlock function.
      *
      * Calling this function will unlock the mutex allowing other threads to acquire it.
      */
    void unlock() const;

    /**
      * @brief Initialization function (virtual).
      *
      * This function will need to be implemented by classes deriving from this.
      *
      * @param[in] weak_ref the weak reference to the object this helper is associated with.
      */
    virtual void init (ObjectDelegate::weak_ref_type weak_ref) = 0;

    /**
      * @brief Weak reference getter.
      *
      * @return ObjectDelegate::weak_ref_type the weak reference to the object associated with this.
      */
    ObjectDelegate::weak_ref_type get_weak_ref () const;

    /**
      * @brief Strong reference getter.
      *
      * Will lock the weak reference this object is associated with, thereby ensuring its lifetime.
      *
      * @return ObjectDelegate::ref_type the reference to the object associated with this.
      */
    ObjectDelegate::ref_type get_strong_ref () const;

protected:

    void check () const;
    void set_weak_ref (ObjectDelegate::weak_ref_type weak_ref);

    Mutex mutex;
    bool closed;
    ObjectDelegate::weak_ref_type myself;
};

DDSCXX_WARNING_MSVC_ON(4251)

}
}
}
}

#endif /* CYCLONEDDS_CORE_OBJECT_DELEGATE_HPP_ */
