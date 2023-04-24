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


/**
 * @file
 */

#ifndef CYCLONEDDS_CORE_POLICY_QOS_DELEGATE_HPP_
#define CYCLONEDDS_CORE_POLICY_QOS_DELEGATE_HPP_

#include <org/eclipse/cyclonedds/core/ReportUtils.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <cassert>
#include "dds/ddsi/ddsi_plist.h"

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

/* this gives the bit mask for the given policy */
template<typename POLICY>
constexpr uint64_t policy_mask();

/* this returns true when a QoSPolicy is supported for a specific QoS */
template <typename QOS, typename POLICY>
constexpr bool supports_policy() { return false; }

/* use this macro to enable a QoSPolicy for a specific QoS type */
#define ENABLE_POLICY(QOS,POLICY)\
template<> \
constexpr bool org::eclipse::cyclonedds::core::policy::supports_policy<QOS,dds::core::policy::POLICY>() { return true; }

/* this exposes the correct member functions for QoSes */
#define MEMBER_FUNCTIONS(QOS)\
template<typename POLICY> void policy(const POLICY& toset) {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    setter<POLICY>(toset);\
}\
template<typename POLICY> const POLICY& policy() const {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    return c_getter<POLICY>();\
}\
template<typename POLICY> POLICY& policy() {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    return getter<POLICY>();\
}\
template <typename POLICY>\
QOS& operator << (const POLICY& p) {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    setter(p);\
    return *this;\
}\
template <typename POLICY>\
const QOS& operator >> (POLICY& p) const {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    p = c_getter<POLICY>();\
    return *this;\
}\
template<typename POLICY> bool is_present() const {\
    static_assert(org::eclipse::cyclonedds::core::policy::supports_policy<QOS,POLICY>(), "Policy is not supported for this type of QoS");\
    return _present & org::eclipse::cyclonedds::core::policy::policy_mask<POLICY>();\
}\
void check() const { QosDelegate::_check(); }\
dds_qos_t* ddsc_qos() const { return QosDelegate::_ddsc_qos(); }\
void ddsc_qos(const dds_qos_t* qos) { QosDelegate::_ddsc_qos(qos); } \
template<typename DERIVED> bool operator==(const DERIVED& other) const { return QosDelegate::compare(*reinterpret_cast<const QosDelegate*>(&other)); }\
template<typename DERIVED> bool operator!=(const DERIVED& other) const { return !QosDelegate::compare(*reinterpret_cast<const QosDelegate*>(&other)); }\
template<typename DERIVED> QOS& operator=(const DERIVED& other) { merge(*reinterpret_cast<const QosDelegate*>(&other), true); return *this; }\
template<typename DERIVED> QOS(const DERIVED& other): QOS() { *this = *reinterpret_cast<const QosDelegate*>(&other); }\
QOS();

class OMG_DDS_API QosDelegate
{
public:
    MEMBER_FUNCTIONS(QosDelegate)
protected:
    bool compare(const QosDelegate& other) const;
    void merge(const QosDelegate& other, bool update_only);
    /* these accessor functions are implemented using OMG_DDS_IMPLEMENT_POLICY*/
    template<typename POLICY> void setter(const POLICY& toset);
    template<typename POLICY> const POLICY& c_getter() const;
    template<typename POLICY> POLICY& getter();

    /* The returned ddsc QoS has to be freed. */
    dds_qos_t* _ddsc_qos() const;
    void _ddsc_qos(const dds_qos_t* qos);
    void _check() const;
    uint64_t _present = 0;
private:
    /* this implements the members for the indicated policy */
#define OMG_DDS_DEFINE_POLICY(POLICY) \
    dds::core::policy::POLICY POLICY;

    OMG_DDS_DEFINE_POLICY(UserData)
    OMG_DDS_DEFINE_POLICY(Durability)
    OMG_DDS_DEFINE_POLICY(Presentation)
    OMG_DDS_DEFINE_POLICY(Deadline)
    OMG_DDS_DEFINE_POLICY(LatencyBudget)
    OMG_DDS_DEFINE_POLICY(Ownership)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_DEFINE_POLICY(OwnershipStrength)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_DEFINE_POLICY(Liveliness)
    OMG_DDS_DEFINE_POLICY(TimeBasedFilter)
    OMG_DDS_DEFINE_POLICY(Partition)
    OMG_DDS_DEFINE_POLICY(Reliability)
    OMG_DDS_DEFINE_POLICY(DestinationOrder)
    OMG_DDS_DEFINE_POLICY(History)
    OMG_DDS_DEFINE_POLICY(ResourceLimits)
    OMG_DDS_DEFINE_POLICY(EntityFactory)
    OMG_DDS_DEFINE_POLICY(WriterDataLifecycle)
    OMG_DDS_DEFINE_POLICY(ReaderDataLifecycle)
    OMG_DDS_DEFINE_POLICY(TopicData)
    OMG_DDS_DEFINE_POLICY(GroupData)
    OMG_DDS_DEFINE_POLICY(TransportPriority)
    OMG_DDS_DEFINE_POLICY(Lifespan)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    OMG_DDS_DEFINE_POLICY(DurabilityService)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    OMG_DDS_DEFINE_POLICY(DataRepresentation)
    OMG_DDS_DEFINE_POLICY(TypeConsistencyEnforcement)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef OMG_DDS_DEFINE_POLICY
};

/* this implements the protected getters/setters for the QoSPolicy in the QosDelegate class */
#define OMG_DDS_IMPLEMENT_POLICY(POLICY,C_MASK) \
    template<> constexpr uint64_t \
    policy_mask<dds::core::policy::POLICY>() { return C_MASK; } \
    template<> inline void QosDelegate::setter(const dds::core::policy::POLICY& toset) {\
        toset.delegate().check();\
        _present |= policy_mask<dds::core::policy::POLICY>();\
        POLICY = toset;\
    }\
    template<> inline const dds::core::policy::POLICY&  QosDelegate::c_getter<dds::core::policy::POLICY>() const {\
        assert(is_present<dds::core::policy::POLICY>());\
        return POLICY;\
    } \
    template<> inline dds::core::policy::POLICY& QosDelegate::getter<dds::core::policy::POLICY>() {\
        _present |= policy_mask<dds::core::policy::POLICY>();\
        return POLICY;\
    }\
    template<> constexpr bool supports_policy<QosDelegate,dds::core::policy::POLICY>() { return true; }

    OMG_DDS_IMPLEMENT_POLICY(UserData, DDSI_QP_USER_DATA)
    OMG_DDS_IMPLEMENT_POLICY(Durability, DDSI_QP_DURABILITY)
    OMG_DDS_IMPLEMENT_POLICY(Presentation, DDSI_QP_PRESENTATION)
    OMG_DDS_IMPLEMENT_POLICY(Deadline, DDSI_QP_DEADLINE)
    OMG_DDS_IMPLEMENT_POLICY(LatencyBudget, DDSI_QP_LATENCY_BUDGET)
    OMG_DDS_IMPLEMENT_POLICY(Ownership, DDSI_QP_OWNERSHIP)
    #ifdef OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_IMPLEMENT_POLICY(OwnershipStrength, DDSI_QP_OWNERSHIP_STRENGTH)
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    OMG_DDS_IMPLEMENT_POLICY(Liveliness, DDSI_QP_LIVELINESS)
    OMG_DDS_IMPLEMENT_POLICY(TimeBasedFilter, DDSI_QP_TIME_BASED_FILTER)
    OMG_DDS_IMPLEMENT_POLICY(Partition, DDSI_QP_PARTITION)
    OMG_DDS_IMPLEMENT_POLICY(Reliability, DDSI_QP_RELIABILITY)
    OMG_DDS_IMPLEMENT_POLICY(DestinationOrder, DDSI_QP_DESTINATION_ORDER)
    OMG_DDS_IMPLEMENT_POLICY(History, DDSI_QP_HISTORY)
    OMG_DDS_IMPLEMENT_POLICY(ResourceLimits, DDSI_QP_RESOURCE_LIMITS)
    OMG_DDS_IMPLEMENT_POLICY(EntityFactory, DDSI_QP_ADLINK_ENTITY_FACTORY)
    OMG_DDS_IMPLEMENT_POLICY(WriterDataLifecycle, DDSI_QP_ADLINK_WRITER_DATA_LIFECYCLE)
    OMG_DDS_IMPLEMENT_POLICY(ReaderDataLifecycle, DDSI_QP_ADLINK_READER_DATA_LIFECYCLE)
    OMG_DDS_IMPLEMENT_POLICY(TopicData, DDSI_QP_TOPIC_DATA)
    OMG_DDS_IMPLEMENT_POLICY(GroupData, DDSI_QP_GROUP_DATA)
    OMG_DDS_IMPLEMENT_POLICY(TransportPriority, DDSI_QP_TRANSPORT_PRIORITY)
    OMG_DDS_IMPLEMENT_POLICY(Lifespan, DDSI_QP_LIFESPAN)
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    OMG_DDS_IMPLEMENT_POLICY(DurabilityService, DDSI_QP_DURABILITY_SERVICE)
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    OMG_DDS_IMPLEMENT_POLICY(DataRepresentation, DDSI_QP_DATA_REPRESENTATION)
    OMG_DDS_IMPLEMENT_POLICY(TypeConsistencyEnforcement, DDSI_QP_TYPE_CONSISTENCY_ENFORCEMENT)
    #endif  // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

#undef OMG_DDS_IMPLEMENT_POLICY

}
}
}
}
}

#endif /* CYCLONEDDS_CORE_POLICY_QOS_DELEGATE_HPP_ */