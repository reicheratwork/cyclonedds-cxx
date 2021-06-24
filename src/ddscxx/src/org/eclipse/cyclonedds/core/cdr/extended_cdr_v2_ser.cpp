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

#include <org/eclipse/cyclonedds/core/cdr/extended_cdr_v2_ser.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

bool xcdr_v2_stream::structure_is_list(extensibility ext) const
{
  return ext == ext_mutable;
}

entity_properties xcdr_v2_stream::read_header()
{
  entity_properties props;

  if (abort_status())
    return props;

  

  return props;
}

void xcdr_v2_stream::push_entity(const entity_properties &props)
{
  if (props.member_extensibility == ext_appendable ||
      props.member_extensibility == ext_mutable)
    write_d_header(props);

  if (props.parent_extensibility == ext_mutable)
    write_em_header(props);
}

void xcdr_v2_stream::pop_entity()
{
  assert(m_headers.size() >= 2);

  if (m_headers.top().parent_extensibility == ext_mutable)
    finish_em_header();

  if (m_headers.top().member_extensibility == ext_appendable ||
      m_headers.top().member_extensibility == ext_mutable)
    finish_d_header();
}

void xcdr_v2_stream::move_header(const entity_properties &props)
{
  (void) props;

  if (abort_status())
    return;
}


void xcdr_v2_stream::write_d_header(const entity_properties &props)
{
  if (abort_status())
    return;

  m_headers.push(props);
  m_headers.top().offset = position();

  uint32_t dheader(0);
  write(*this,dheader);
}

void xcdr_v2_stream::write_em_header(const entity_properties &props)
{
  if (abort_status())
    return;

  m_headers.push(props);
  m_headers.top().offset = position();

  uint32_t mheader = (props.must_understand ? must_understand : 0) + (id_mask & props.member_id);
  switch (m_headers.top().member_fixed_size)
  {
    case one:
      mheader += bytes_1;
      break;
    case two:
      mheader += bytes_2;
      break;
    case four:
      mheader += bytes_4;
      break;
    case eight:
      mheader += bytes_8;
      break;
    default:
      mheader += nextint;
  }
  write(*this, mheader);

  if (m_headers.top().member_fixed_size == other)
    write(*this, uint32_t(0));
}

void xcdr_v2_stream::finish_d_header()
{
  if (abort_status())
    return;

  assert(m_headers.size());

  size_t header_pos = m_headers.top().offset, current_pos = position();
  size_t current_alignment = alignment();
  alignment(4);
  position(header_pos);
  write(*this,static_cast<uint32_t>(current_pos-header_pos-4));

  alignment(current_alignment);
  position(current_pos);

  m_headers.pop();
}

void xcdr_v2_stream::finish_em_header()
{
  if (abort_status())
    return;

  assert(m_headers.size());

  if (m_headers.top().member_fixed_size == other)
  {
    size_t header_pos = m_headers.top().offset, current_pos = position();
    size_t current_alignment = alignment();
    alignment(4);
    position(header_pos+4);
    write(*this,static_cast<uint32_t>(current_pos-header_pos-8));

    alignment(current_alignment);
    position(current_pos);
  }

  m_headers.pop();
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
