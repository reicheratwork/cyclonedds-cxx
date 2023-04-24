/*
 * Copyright(c) 2023 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace core
{
namespace policy
{

QosDelegate::QosDelegate()
{
}

void QosDelegate::_ddsc_qos(const dds_qos_t* qos)
{
    assert(qos);

#define OMG_DDS_CHECK_AND_SET_ISO(POLICY) \
    if (qos->present & policy_mask<dds::core::policy::POLICY>()) {\
        _present |= policy_mask<dds::core::policy::POLICY>();\
        policy<dds::core::policy::POLICY>().delegate().set_iso_policy(qos);\
    }

    OMG_DDS_CHECK_AND_SET_ISO(UserData)
    OMG_DDS_CHECK_AND_SET_ISO(Durability)
    OMG_DDS_CHECK_AND_SET_ISO(Presentation)
    OMG_DDS_CHECK_AND_SET_ISO(Deadline)
    OMG_DDS_CHECK_AND_SET_ISO(Ownership)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_CHECK_AND_SET_ISO(OwnershipStrength)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_CHECK_AND_SET_ISO(Liveliness)
    OMG_DDS_CHECK_AND_SET_ISO(TimeBasedFilter)
    OMG_DDS_CHECK_AND_SET_ISO(Partition)
    OMG_DDS_CHECK_AND_SET_ISO(Reliability)
    OMG_DDS_CHECK_AND_SET_ISO(DestinationOrder)
    OMG_DDS_CHECK_AND_SET_ISO(History)
    OMG_DDS_CHECK_AND_SET_ISO(ResourceLimits)
    OMG_DDS_CHECK_AND_SET_ISO(EntityFactory)
    OMG_DDS_CHECK_AND_SET_ISO(WriterDataLifecycle)
    OMG_DDS_CHECK_AND_SET_ISO(ReaderDataLifecycle)
    OMG_DDS_CHECK_AND_SET_ISO(TopicData)
    OMG_DDS_CHECK_AND_SET_ISO(GroupData)
    OMG_DDS_CHECK_AND_SET_ISO(TransportPriority)
    OMG_DDS_CHECK_AND_SET_ISO(Lifespan)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    OMG_DDS_CHECK_AND_SET_ISO(DurabilityService)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    OMG_DDS_CHECK_AND_SET_ISO(DataRepresentation)
    OMG_DDS_CHECK_AND_SET_ISO(TypeConsistencyEnforcement)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef OMG_DDS_CHECK_AND_SET_ISO
}

dds_qos_t* QosDelegate::_ddsc_qos() const
{
    dds_qos_t* qos = dds_create_qos();
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }

#define OMG_DDS_CHECK_AND_SET_C(POLICY) \
    if (is_present<dds::core::policy::POLICY>())\
        POLICY.delegate().set_c_policy(qos);

    OMG_DDS_CHECK_AND_SET_C(UserData)
    OMG_DDS_CHECK_AND_SET_C(Durability)
    OMG_DDS_CHECK_AND_SET_C(Presentation)
    OMG_DDS_CHECK_AND_SET_C(Deadline)
    OMG_DDS_CHECK_AND_SET_C(Ownership)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_CHECK_AND_SET_C(OwnershipStrength)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_CHECK_AND_SET_C(Liveliness)
    OMG_DDS_CHECK_AND_SET_C(TimeBasedFilter)
    OMG_DDS_CHECK_AND_SET_C(Partition)
    OMG_DDS_CHECK_AND_SET_C(Reliability)
    OMG_DDS_CHECK_AND_SET_C(DestinationOrder)
    OMG_DDS_CHECK_AND_SET_C(History)
    OMG_DDS_CHECK_AND_SET_C(ResourceLimits)
    OMG_DDS_CHECK_AND_SET_C(EntityFactory)
    OMG_DDS_CHECK_AND_SET_C(WriterDataLifecycle)
    OMG_DDS_CHECK_AND_SET_C(ReaderDataLifecycle)
    OMG_DDS_CHECK_AND_SET_C(TopicData)
    OMG_DDS_CHECK_AND_SET_C(GroupData)
    OMG_DDS_CHECK_AND_SET_C(TransportPriority)
    OMG_DDS_CHECK_AND_SET_C(Lifespan)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    OMG_DDS_CHECK_AND_SET_C(DurabilityService)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    OMG_DDS_CHECK_AND_SET_C(DataRepresentation)
    OMG_DDS_CHECK_AND_SET_C(TypeConsistencyEnforcement)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef OMG_DDS_CHECK_AND_SET_C

    return qos;
}

void QosDelegate::_check() const
{
    if (is_present<dds::core::policy::History>() &&
        is_present<dds::core::policy::ResourceLimits>())
        History.delegate().check_against(ResourceLimits.delegate());
    if (is_present<dds::core::policy::Deadline>() &&
        is_present<dds::core::policy::TimeBasedFilter>())
        Deadline.delegate().check_against(TimeBasedFilter.delegate());
}

bool QosDelegate::compare(const QosDelegate& other) const {

#define CHECK_AND_COMPARE(POLICY) \
    if (is_present<dds::core::policy::POLICY>() != other.is_present<dds::core::policy::POLICY>() || \
        (is_present<dds::core::policy::POLICY>() && \
         policy<dds::core::policy::POLICY>() !=  other.policy<dds::core::policy::POLICY>()))\
        return false;

    CHECK_AND_COMPARE(UserData)
    CHECK_AND_COMPARE(Durability)
    CHECK_AND_COMPARE(Presentation)
    CHECK_AND_COMPARE(Deadline)
    CHECK_AND_COMPARE(Ownership)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    CHECK_AND_COMPARE(OwnershipStrength)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    CHECK_AND_COMPARE(Liveliness)
    CHECK_AND_COMPARE(TimeBasedFilter)
    CHECK_AND_COMPARE(Partition)
    CHECK_AND_COMPARE(Reliability)
    CHECK_AND_COMPARE(DestinationOrder)
    CHECK_AND_COMPARE(History)
    CHECK_AND_COMPARE(ResourceLimits)
    CHECK_AND_COMPARE(EntityFactory)
    CHECK_AND_COMPARE(WriterDataLifecycle)
    CHECK_AND_COMPARE(ReaderDataLifecycle)
    CHECK_AND_COMPARE(TopicData)
    CHECK_AND_COMPARE(GroupData)
    CHECK_AND_COMPARE(TransportPriority)
    CHECK_AND_COMPARE(Lifespan)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    CHECK_AND_COMPARE(DurabilityService)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    CHECK_AND_COMPARE(DataRepresentation)
    CHECK_AND_COMPARE(TypeConsistencyEnforcement)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef CHECK_AND_COMPARE

    return true;
}

void QosDelegate::merge(const QosDelegate &other, bool update_only)
{
#define MERGE(POLICY) \
    if (other.is_present<dds::core::policy::POLICY>() && \
        (is_present<dds::core::policy::POLICY>() || !update_only)) \
        setter(other.POLICY);

    MERGE(UserData)
    MERGE(Durability)
    MERGE(Presentation)
    MERGE(Deadline)
    MERGE(Ownership)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    MERGE(OwnershipStrength)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    MERGE(Liveliness)
    MERGE(TimeBasedFilter)
    MERGE(Partition)
    MERGE(Reliability)
    MERGE(DestinationOrder)
    MERGE(History)
    MERGE(ResourceLimits)
    MERGE(EntityFactory)
    MERGE(WriterDataLifecycle)
    MERGE(ReaderDataLifecycle)
    MERGE(TopicData)
    MERGE(GroupData)
    MERGE(TransportPriority)
    MERGE(Lifespan)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    MERGE(DurabilityService)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    MERGE(DataRepresentation)
    MERGE(TypeConsistencyEnforcement)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef MERGE
    check();
}


}
}
}
}
}