/*
 * Copyright(c) 2020 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <cstring>
#include <algorithm>
#include <assert.h>

#include <org/eclipse/cyclonedds/core/cdr/cdr_stream.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

entity_properties_t cdr_stream::m_final = final_entry();

void cdr_stream::set_buffer(void* toset) {
  m_buffer = static_cast<char*>(toset);
  reset_position();
}

size_t cdr_stream::align(size_t newalignment, bool add_zeroes)
{
  if (m_current_alignment == newalignment)
    return 0;

  m_current_alignment = std::min(newalignment, m_max_alignment);

  size_t tomove = (m_current_alignment - m_position % m_current_alignment) % m_current_alignment;
  if (tomove && add_zeroes && m_buffer) {
    auto cursor = get_cursor();
    assert(cursor);
    memset(cursor, 0, tomove);
  }

  m_position += tomove;

  return tomove;
}

entity_properties_t& cdr_stream::next_prop(entity_properties_t &props, bool as_key, stream_mode mode, bool &firstcall)
{
  if (firstcall) {
    auto it = as_key ?
              props.m_keys_by_seq.begin() :
              props.m_members_by_seq.begin();
    m_stack.push(it);
    start_member(it, mode);
    firstcall = false;
    return *it;
  }

  assert(m_stack.size());

  auto &it = m_stack.top();
  if (*it)
    finish_member(it++, mode);

  entity_properties_t &prop = *it;
  if (prop)
    //the current entry is valid
    start_member(it, mode);
  else
    //the end of the current level of the list has been reached
    m_stack.pop();

  return prop;
}

uint64_t cdr_stream::props_to_id(const entity_properties_t &props)
{
  assert(props);
  return (static_cast<uint64_t>(props.m_id)<<32) + props.s_id;
}

entity_properties_t& cdr_stream::top_of_stack()
{
  assert(m_stack.size());
  return *(m_stack.top());
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
