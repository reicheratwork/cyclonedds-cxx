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

#ifndef CYCLONEDDS_CORE_ENTITY_DELEGATE_HPP_
#define CYCLONEDDS_CORE_ENTITY_DELEGATE_HPP_

#include <dds/core/status/State.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <org/eclipse/cyclonedds/core/DDScObjectDelegate.hpp>
#include <org/eclipse/cyclonedds/ForwardDeclarations.hpp>
#include <org/eclipse/cyclonedds/core/status/StatusDelegate.hpp>

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace core
{

/**
  * @brief Entity Delegate implementation class.
  *
  * Due to an expansion of the OMG DDS C++ spec, the functions of this delegate are accessible through the
  * use of the derefencing operators (* and ->).
  */
class OMG_DDS_API EntityDelegate :
    public virtual ::org::eclipse::cyclonedds::core::DDScObjectDelegate
{
public:
    /**
      * @brief Convenience typedef.
      */
    typedef ::dds::core::smart_ptr_traits< EntityDelegate >::ref_type ref_type;
    /**
      * @brief Convenience typedef.
      */
    typedef ::dds::core::smart_ptr_traits< EntityDelegate >::weak_ref_type weak_ref_type;

    /**
      * @brief Default constructor.
      */
    EntityDelegate();
    /**
      * @brief Destructor (virtual).
      */
    virtual ~EntityDelegate();

    /**
      * @brief Enables this entity.
      *
      * Prior to enabling, the entity is not yet published on the DDS service and QoSPolicies
      * that are not Changeable can still be modified.
      */
    void enable();

    /**
      * @brief Status changes getter.
      *
      * Will retrieve the statuses that have changed since the last time statuses have been retrieved.
      *
      * @return dds::core::status::StatusMask the statuses that have changed.
      */
    ::dds::core::status::StatusMask status_changes() const;

    /**
      * @brief Instance handle getter.
      *
      * Will return the instance handle of this entity.
      *
      * @return dds::core::InstanceHandle the instance handle of this entity.
      */
    ::dds::core::InstanceHandle instance_handle() const;

    /**
      * @brief Entity subordination check function.
      *
      * Will return whether the supplied entity is subordinated to this entity.
      * Currently not implemented!!!
      *
      * @param handle the entity to check for.
      *
      * @return bool whether the supplied entity is subordinated to this.
      */
    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    /**
      * @brief Statuscondition getter.
      *
      * Returns the StatusConditionDelegate associated with this entity.
      *
      * @return ObjectDelegate::ref_type the StatusConditionDelegate.
      */
    ObjectDelegate::ref_type get_statusCondition();

    /**
      * @brief Close function.
      *
      * Will remove this entity from the DDS space.
      * This function precedes the actual destruction of the entity.
      */
    virtual void close();

    /**
      * @brief Retain function.
      *
      * Currently not implemented!!!
      */
    virtual void retain();

    /**
      * @brief Listener getter function.
      *
      * Returns an anonymous pointer to any listener associated with this entity.
      * This pointer needs to be converted to the class specific listener for this
      * entity before it can be used. E.g.: if this entity is a DataReader, then
      * this pointer should be converted to DataReaderListener*.
      *
      * @return void* the pointer to any listener associated with this entity.
      */
    void *listener_get() const;

protected:
    void listener_set(void *listener,
            const dds::core::status::StatusMask& mask);

public:
    /**
      * @brief Listener mask getter function.
      *
      * Returns the bitmask for the status changes the associated listener should do callbacks on.
      *
      * @return dds::core::status::StatusMask the listener mask associated with this entity.
      */
    const dds::core::status::StatusMask get_listener_mask() const ;

    /**
      * @brief Callback function queuer.
      *
      * Will increase the number of queued callback functions by 1.
      *
      * @return bool Whether there were already callback functions queued for this entity.
      */
    bool obtain_callback_lock() ;

    /**
      * @brief Callback function dequeuer.
      *
      * Will decrease the number of queued callback functions by 1.
      */
    void release_callback_lock() ;

    // Topic callback
    /**
      * @brief Callback function for on_inconsistent_topic status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, Topic). This function should never
      * be called itself.
      *
      * @param topic The CycloneDDS-C topic entity triggering this callback.
      */
    virtual void on_inconsistent_topic(dds_entity_t topic,
          org::eclipse::cyclonedds::core::InconsistentTopicStatusDelegate &) ;


    // Writer callbacks
    /**
      * @brief Callback function for on_offered_deadline_missed status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataWriter). This function should never
      * be called itself.
      *
      * @param writer The CycloneDDS-C writer entity triggering this callback.
      */
    virtual void on_offered_deadline_missed(dds_entity_t writer,
          org::eclipse::cyclonedds::core::OfferedDeadlineMissedStatusDelegate &) ;
    /**
      * @brief Callback function for on_offered_incompatible_qos status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataWriter). This function should never
      * be called itself.
      *
      * @param writer The CycloneDDS-C writer entity triggering this callback.
      */
    virtual void on_offered_incompatible_qos(dds_entity_t writer,
          org::eclipse::cyclonedds::core::OfferedIncompatibleQosStatusDelegate &) ;
    /**
      * @brief Callback function for on_liveliness_lost status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataWriter). This function should never
      * be called itself.
      *
      * @param writer The CycloneDDS-C writer entity triggering this callback.
      */
    virtual void on_liveliness_lost(dds_entity_t writer,
          org::eclipse::cyclonedds::core::LivelinessLostStatusDelegate &) ;
    /**
      * @brief Callback function for on_publication_matched status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataWriter). This function should never
      * be called itself.
      *
      * @param writer The CycloneDDS-C writer entity triggering this callback.
      */
    virtual void on_publication_matched(dds_entity_t writer,
          org::eclipse::cyclonedds::core::PublicationMatchedStatusDelegate &) ;


    // Reader callbacks
    /**
      * @brief Callback function for on_requested_deadline_missed status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_requested_deadline_missed(dds_entity_t reader,
          org::eclipse::cyclonedds::core::RequestedDeadlineMissedStatusDelegate &);
    /**
      * @brief Callback function for on_requested_incompatible_qos status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_requested_incompatible_qos(dds_entity_t reader,
          org::eclipse::cyclonedds::core::RequestedIncompatibleQosStatusDelegate &);
    /**
      * @brief Callback function for on_sample_rejected status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_sample_rejected(dds_entity_t reader,
          org::eclipse::cyclonedds::core::SampleRejectedStatusDelegate &);
    /**
      * @brief Callback function for on_liveliness_changed status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_liveliness_changed(dds_entity_t reader,
          org::eclipse::cyclonedds::core::LivelinessChangedStatusDelegate &);
    /**
      * @brief Callback function for on_data_available status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_data_available(dds_entity_t reader);
    /**
      * @brief Callback function for on_subscription_matched status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_subscription_matched(dds_entity_t reader,
          org::eclipse::cyclonedds::core::SubscriptionMatchedStatusDelegate &);
    /**
      * @brief Callback function for on_sample_lost status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, DataReader). This function should never
      * be called itself.
      *
      * @param reader The CycloneDDS-C reader entity triggering this callback.
      */
    virtual void on_sample_lost(dds_entity_t reader,
          org::eclipse::cyclonedds::core::SampleLostStatusDelegate &);


    // Subscriber callback
    /**
      * @brief Callback function for on_data_readers status.
      *
      * This is only a dummy function and should be overridden by the entity's
      * real implementation (in this case, Subscriber). This function should never
      * be called itself.
      *
      * @param subscriber The CycloneDDS-C subscriber entity triggering this callback.
      */
    virtual void on_data_readers(dds_entity_t subscriber);

protected:
    static volatile unsigned int entityID_;
    bool enabled_;
    dds::core::status::StatusMask listener_mask;
    void prevent_callbacks();
    long callback_count;
    dds_listener_t *listener_callbacks;

private:
    void *listener;
    ObjectDelegate::weak_ref_type myStatusCondition;
    void *callback_mutex;
    void *callback_cond;
};

}
}
}
}

#endif /* CYCLONEDDS_CORE_ENTITY_DELEGATE_HPP_ */
