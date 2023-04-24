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

#include <org/eclipse/cyclonedds/domain/qos/DomainParticipantQosDelegate.hpp>

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace domain
{
namespace qos
{

constexpr uint64_t mask =
    core::policy::policy_mask<dds::core::policy::UserData>() |
    core::policy::policy_mask<dds::core::policy::EntityFactory>();

DomainParticipantQosDelegate::DomainParticipantQosDelegate()
{
    ddsc_qos(&ddsi_default_qos_participant);
    _present = mask;
    check();
}

}
}
}
}
}
