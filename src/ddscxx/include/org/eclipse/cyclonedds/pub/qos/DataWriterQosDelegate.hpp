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

#ifndef CYCLONEDDS_PUB_QOS_DATA_WRITER_QOS_DELEGATE_HPP_
#define CYCLONEDDS_PUB_QOS_DATA_WRITER_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>
#include <org/eclipse/cyclonedds/topic/qos/TopicQosDelegate.hpp>

struct _DDS_NamedDataWriterQos;

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace pub
{
namespace qos
{

class OMG_DDS_API DataWriterQosDelegate: protected core::policy::QosDelegate
{
public:
    void named_qos(const struct _DDS_NamedDataWriterQos &) {;}

    MEMBER_FUNCTIONS(DataWriterQosDelegate)
};

}
}

ENABLE_POLICY(pub::qos::DataWriterQosDelegate, UserData)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Durability)
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, DurabilityService)
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Deadline)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, LatencyBudget)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Liveliness)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Reliability)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, DestinationOrder)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, History)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, ResourceLimits)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, TransportPriority)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Lifespan)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, Ownership)
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, OwnershipStrength)
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, WriterDataLifecycle)
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, DataRepresentation)
ENABLE_POLICY(pub::qos::DataWriterQosDelegate, TypeConsistencyEnforcement)
#endif //  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

}
}
}

#endif /* CYCLONEDDS_PUB_QOS_DATA_WRITER_QOS_DELEGATE_HPP_ */
