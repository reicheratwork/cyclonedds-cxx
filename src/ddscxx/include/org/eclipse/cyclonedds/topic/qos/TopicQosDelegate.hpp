/*
 * Copyright(c) 2006 to 2023 ZettaScale Technology and others
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

#ifndef CYCLONEDDS_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_
#define CYCLONEDDS_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>

struct _DDS_NamedTopicQos;

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace topic
{
namespace qos
{

class OMG_DDS_API TopicQosDelegate: protected core::policy::QosDelegate
{
public:
    void named_qos(const struct _DDS_NamedTopicQos &) {;}

    MEMBER_FUNCTIONS(TopicQosDelegate)
};

}
}

ENABLE_POLICY(topic::qos::TopicQosDelegate, TopicData)
ENABLE_POLICY(topic::qos::TopicQosDelegate, Durability)
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
ENABLE_POLICY(topic::qos::TopicQosDelegate, DurabilityService)
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
ENABLE_POLICY(topic::qos::TopicQosDelegate, Deadline)
ENABLE_POLICY(topic::qos::TopicQosDelegate, LatencyBudget)
ENABLE_POLICY(topic::qos::TopicQosDelegate, Liveliness)
ENABLE_POLICY(topic::qos::TopicQosDelegate, Reliability)
ENABLE_POLICY(topic::qos::TopicQosDelegate, DestinationOrder)
ENABLE_POLICY(topic::qos::TopicQosDelegate, History)
ENABLE_POLICY(topic::qos::TopicQosDelegate, ResourceLimits)
ENABLE_POLICY(topic::qos::TopicQosDelegate, TransportPriority)
ENABLE_POLICY(topic::qos::TopicQosDelegate, Lifespan)
ENABLE_POLICY(topic::qos::TopicQosDelegate, Ownership)
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
ENABLE_POLICY(topic::qos::TopicQosDelegate, DataRepresentation)
ENABLE_POLICY(topic::qos::TopicQosDelegate, TypeConsistencyEnforcement)
#endif //  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

}
}
}

#endif /* CYCLONEDDS_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_ */
