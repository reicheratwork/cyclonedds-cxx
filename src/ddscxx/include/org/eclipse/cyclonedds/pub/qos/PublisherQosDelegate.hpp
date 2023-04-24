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

#ifndef CYCLONEDDS_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_
#define CYCLONEDDS_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/eclipse/cyclonedds/core/policy/QosDelegate.hpp>

struct _DDS_NamedPublisherQos;

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

class OMG_DDS_API PublisherQosDelegate: protected core::policy::QosDelegate
{
public:
    void named_qos(const struct _DDS_NamedPublisherQos &) {;}

    MEMBER_FUNCTIONS(PublisherQosDelegate)
};

}
}

ENABLE_POLICY(pub::qos::PublisherQosDelegate, Presentation)
ENABLE_POLICY(pub::qos::PublisherQosDelegate, Partition)
ENABLE_POLICY(pub::qos::PublisherQosDelegate, GroupData)
ENABLE_POLICY(pub::qos::PublisherQosDelegate, EntityFactory)

}
}
}

#endif /* CYCLONEDDS_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_ */
