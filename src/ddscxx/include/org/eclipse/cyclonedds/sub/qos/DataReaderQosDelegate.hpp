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

#ifndef CYCLONEDDS_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_
#define CYCLONEDDS_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>
#include <org/eclipse/cyclonedds/topic/qos/TopicQosDelegate.hpp>

struct _DDS_NamedDataReaderQos;

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace sub
{
namespace qos
{

class OMG_DDS_API DataReaderQosDelegate: protected core::policy::QosDelegate
{
public:
    void named_qos(const struct _DDS_NamedDataReaderQos &) {;}

    MEMBER_FUNCTIONS(DataReaderQosDelegate)
};

}
}

ENABLE_POLICY(sub::qos::DataReaderQosDelegate, UserData)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, Durability)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, Deadline)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, LatencyBudget)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, Liveliness)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, Reliability)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, DestinationOrder)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, History)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, ResourceLimits)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, Ownership)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, TimeBasedFilter)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, ReaderDataLifecycle)
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, DataRepresentation)
ENABLE_POLICY(sub::qos::DataReaderQosDelegate, TypeConsistencyEnforcement)
#endif //  OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

}
}
}

#endif /* CYCLONEDDS_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_ */
