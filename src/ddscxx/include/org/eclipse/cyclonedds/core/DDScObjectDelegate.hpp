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

#ifndef CYCLONEDDS_CORE_USEROBJECT_DELEGATE_HPP_
#define CYCLONEDDS_CORE_USEROBJECT_DELEGATE_HPP_

#include "dds/core/macros.hpp"
#include "dds/core/refmacros.hpp"
#include "org/eclipse/cyclonedds/core/Mutex.hpp"
#include "org/eclipse/cyclonedds/core/ObjectDelegate.hpp"

#include <unordered_map>

#include "dds/dds.h"

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
  * @brief Wrapper class for CycloneDDS-C entities to CycloneDDS-CXX Objects.
  */
class OMG_DDS_API DDScObjectDelegate : public virtual org::eclipse::cyclonedds::core::ObjectDelegate
{
public:
    /**
      * @brief Convenience typedef.
      */
    typedef std::unordered_map<dds_entity_t,org::eclipse::cyclonedds::core::ObjectDelegate::weak_ref_type> entity_map_type;

    /**
      * @brief Default constructor.
      */
    DDScObjectDelegate ();

    /**
      * @brief Destructor (virtual).
      *
      * Removes this object from the entity map, and calls dds_delete on the CycloneDDS-C entity.
      */
    virtual ~DDScObjectDelegate ();

    /**
      * @brief Close function.
      *
      * Indicates that this entity will no longer participate in the DDS shared dataspace.
      */
    void close ();

    /**
      * @brief CycloneDDS-C entity getter.
      *
      * Returns the CycloneDDS-C entity this object wraps.
      *
      * @return dds_entity_t the entity wrapped by this object.
      */
    dds_entity_t get_ddsc_entity ();

    /**
      * @brief CycloneDDS-C entity setter.
      *
      * Sets the CycloneDDS-C entity this object wraps.
      *
      * @param[in] e the entity to be wrapped by this object.
      */
    void set_ddsc_entity (dds_entity_t e);

    /**
      * @brief Entity management addition function.
      *
      * Adds this entity to the map of C to C++ entities.
      * This entity will be removed from this mapping on deletion.
      *
      * @param[in] weak_ref reference to the entity to add to the mapping.
      */
    void add_to_entity_map (org::eclipse::cyclonedds::core::ObjectDelegate::weak_ref_type weak_ref);

public:

    /**
      * @brief C to C++ conversion function.
      *
      * Looks up the supplied C entity in the entity management mapping, and
      * returns the associated C++ reference, if any.
      *
      * @param[in] e the CycloneDDS-C entity to look up.
      *
      * @return ObjectDelegate::ref_type strong reference to the C++ entity that wraps e.
      */
    static ObjectDelegate::ref_type extract_strong_ref(dds_entity_t e);

protected:
    dds_entity_t ddsc_entity;

private:
    void delete_from_entity_map();
    static org::eclipse::cyclonedds::core::DDScObjectDelegate::entity_map_type entity_map;
    static Mutex entity_map_mutex;
};

DDSCXX_WARNING_MSVC_ON(4251)

}
}
}
}

#endif /* CYCLONEDDS_CORE_USEROBJECT_DELEGATE_HPP_ */
