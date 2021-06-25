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

entity_properties xcdr_v2_stream::read_header()
{
  entity_properties props;

  uint32_t emheader = 0;
  read(*this,emheader);

  uint32_t factor = 0;
  props.must_understand = emheader & must_understand;
  props.member_id = emheader & id_mask;
  switch (emheader & lc_mask)
  {
    case bytes_1:
      props.member_length = 1;
      break;
    case bytes_2:
      props.member_length = 2;
      break;
    case bytes_4:
      props.member_length = 4;
      break;
    case bytes_8:
      props.member_length = 8;
      break;
    case nextint:
    case nextint_times_1:
      factor = 1;
      break;
    case nextint_times_4:
      factor = 4;
      break;
    case nextint_times_8:
      factor = 8;
      break;
  }

  if (factor)
  {
    uint32_t next_int = 0;
    read(*this,next_int);
    props.member_length = factor * next_int;
  }

  return props;
}

void xcdr_v2_stream::push_entity(const entity_properties &props)
{
  if (em_header_necessary(props))
    write_em_header(props);
}

void xcdr_v2_stream::pop_entity(const entity_properties &props)
{
  if (em_header_necessary(props))
    finish_em_header();
}

void xcdr_v2_stream::write_d_header(const entity_properties &props)
{
  m_headers.push(props);
  m_headers.top().offset = position();

  write(*this, uint32_t(0));
}

void xcdr_v2_stream::write_em_header(const entity_properties &props, size_t N)
{
  uint32_t mheader = (props.must_understand ? must_understand : 0)
                     + (id_mask & props.member_id) + nextint;

  write(*this, mheader);
  if (N == 0) {
    m_headers.push(props);
    m_headers.top().offset = position();
  }
  write(*this, static_cast<uint32_t>(N));
}

void xcdr_v2_stream::finish_d_header()
{
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

bool xcdr_v2_stream::d_header_necessary(const entity_properties &props)
{
  return props.entity_extensibility == ext_appendable
      || props.entity_extensibility == ext_mutable;
}

bool xcdr_v2_stream::em_header_necessary(const entity_properties &props)
{
  return props.parent_extensibility == ext_mutable;
}

void xcdr_v2_stream::move_d_header()
{
  move(*this,uint32_t());
}

void xcdr_v2_stream::move_entity(const entity_properties &props)
{
  if (!em_header_necessary(props))
    return;

  move(*this,uint32_t());
  move(*this,uint32_t());
}

void xcdr_v2_stream::open_struct(const entity_properties &props)
{
  if (d_header_necessary(props))
    write_d_header(props);
}

void xcdr_v2_stream::close_struct(const entity_properties &props)
{
  if (d_header_necessary(props))
    finish_d_header();
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
