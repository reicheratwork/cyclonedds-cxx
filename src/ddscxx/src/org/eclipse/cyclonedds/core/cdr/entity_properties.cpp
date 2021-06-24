/*
 * Copyright(c) 2021 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include <org/eclipse/cyclonedds/core/cdr/extended_cdr_v1_ser.hpp>
#include <iostream>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

bool entity_properties::member_id_comp(const entity_properties_t &lhs, const entity_properties_t &rhs)
{
  if (!rhs && lhs)
    return true;
  if (rhs && !lhs)
    return false;

  return lhs.m_id < rhs.m_id;
}

void entity_properties::print(bool recurse, size_t depth, const char *prefix) const
{
  for (size_t i = 0; i < depth; i++) std::cout << "  ";
  std::cout << prefix << ": s_id: " << s_id << " m_id: " << m_id << " final: " << (is_last ? "yes" : "no") << std::endl;
  if (recurse) {
    for (const auto & e:m_members_by_seq) e.print(true, depth+1, "member");
    for (const auto & e:m_keys_by_seq) e.print(true, depth+1, "key   ");
  }
}

void entity_properties::set_member_props(uint32_t sequence_id, uint32_t member_id, bool optional)
{
  s_id = sequence_id;
  m_id = member_id;
  is_optional = optional;
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
