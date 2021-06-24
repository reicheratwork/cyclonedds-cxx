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

#include <org/eclipse/cyclonedds/core/cdr/extended_cdr_v1_ser.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

bool xcdr_v1_stream::structure_is_list(extensibility ext) const
{
  return ext == ext_mutable;
}

entity_properties xcdr_v1_stream::read_header()
{
  entity_properties props;

  if (abort_status())
    return props;

  align(4, false);


  uint16_t smallid, smalllength;

  read(*this, smallid);
  read(*this, smalllength);

  switch (smallid & pid_mask)
  {
    case pid_list_end:
      props.is_last = true;
      break;
    case pid_ignore:
      props.ignore = true;
      break;
    case pid_extended:
    {
      uint32_t memberheader, largelength;
      read(*this, memberheader);
      read(*this, largelength);

      props.must_understand = pl_extended_flag_must_understand & memberheader;
      props.implementation_extension = pl_extended_flag_impl_extension & memberheader;
      props.member_id = pl_extended_mask & memberheader;
      props.member_length = smalllength;
    }
      break;
    default:
      props.must_understand = pid_flag_must_understand & smallid;
      props.implementation_extension = pid_flag_impl_extension & smallid;
      props.member_id = smallid & pid_mask;
      props.member_length = smalllength;
      if (props.member_id > pid_ignore &&
          props.member_id <= pid_mask)
            status(invalid_pl_entry);
  }

  return props;
}

void xcdr_v1_stream::push_entity(const entity_properties &props)
{
  write_header_fixed(props, 0);

  m_headers.push(props);
  m_headers.top().offset = position()-24;
}

void xcdr_v1_stream::write_header_fixed(const entity_properties &props, size_t N)
{
  if (abort_status())
    return;

  align(4, true);

  uint16_t smallid = pid_extended | pid_flag_must_understand, smalllength = 8;
  uint32_t largeid = props.member_id & pl_extended_mask;

  write(*this, smallid);
  write(*this, smalllength);
  write(*this, largeid);
  write(*this, static_cast<uint32_t>(N));
}

void xcdr_v1_stream::pop_entity()
{
  if (abort_status())
    return;

  assert(m_headers.size());

  size_t end_of_entity_position = position();
  size_t end_of_entity_alignment = m_current_alignment;
  position(m_headers.top().offset+20);
  m_headers.pop();

  uint32_t parlength = static_cast<uint32_t>(end_of_entity_position-position());
  write(*this, parlength);

  position(end_of_entity_position);
  m_current_alignment = end_of_entity_alignment;
}

void xcdr_v1_stream::move_header(const entity_properties &props)
{
  (void) props;

  if (abort_status())
    return;

  align(4, false);

  incr_position(24);
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
