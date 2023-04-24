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

#ifndef CYCLONEDDS_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_
#define CYCLONEDDS_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>

struct _DDS_NamedSubscriberQos;

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

class OMG_DDS_API SubscriberQosDelegate: protected core::policy::QosDelegate
{
public:
    void named_qos(const struct _DDS_NamedSubscriberQos &) {;}

    MEMBER_FUNCTIONS(SubscriberQosDelegate)
};

}
}

ENABLE_POLICY(sub::qos::SubscriberQosDelegate, Presentation)
ENABLE_POLICY(sub::qos::SubscriberQosDelegate, Partition)
ENABLE_POLICY(sub::qos::SubscriberQosDelegate, GroupData)
ENABLE_POLICY(sub::qos::SubscriberQosDelegate, EntityFactory)

}
}
}

#endif /* CYCLONEDDS_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_ */
