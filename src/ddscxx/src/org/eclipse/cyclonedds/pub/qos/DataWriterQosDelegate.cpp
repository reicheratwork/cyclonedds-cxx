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

#include <org/eclipse/cyclonedds/pub/qos/DataWriterQosDelegate.hpp>

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


constexpr uint64_t mask =
    core::policy::policy_mask<dds::core::policy::UserData>() |
    core::policy::policy_mask<dds::core::policy::Durability>() |
    #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    core::policy::policy_mask<dds::core::policy::DurabilityService>() |
    #endif  // OMG_DDS_PERSISTENCE_SUPPORT
    core::policy::policy_mask<dds::core::policy::Deadline>() |
    core::policy::policy_mask<dds::core::policy::LatencyBudget>() |
    core::policy::policy_mask<dds::core::policy::Liveliness>() |
    core::policy::policy_mask<dds::core::policy::Reliability>() |
    core::policy::policy_mask<dds::core::policy::DestinationOrder>() |
    core::policy::policy_mask<dds::core::policy::History>() |
    core::policy::policy_mask<dds::core::policy::ResourceLimits>() |
    core::policy::policy_mask<dds::core::policy::TransportPriority>() |
    core::policy::policy_mask<dds::core::policy::Lifespan>() |
    core::policy::policy_mask<dds::core::policy::Ownership>() |
    #ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    core::policy::policy_mask<dds::core::policy::OwnershipStrength>() |
    #endif  // OMG_DDS_OWNERSHIP_SUPPORT
    core::policy::policy_mask<dds::core::policy::WriterDataLifecycle>()
    #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    | core::policy::policy_mask<dds::core::policy::DataRepresentation>()
    | core::policy::policy_mask<dds::core::policy::TypeConsistencyEnforcement>()
    #endif //  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    ;

DataWriterQosDelegate::DataWriterQosDelegate()
{
    ddsc_qos(&ddsi_default_qos_writer);
    _present = mask;
    policy<dds::core::policy::DataRepresentation>() = dds::core::policy::DataRepresentation();
    check();
}

}
}
}
}
}
