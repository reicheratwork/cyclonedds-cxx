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

#include <org/eclipse/cyclonedds/core/cdr/basic_cdr_ser.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

entity_properties basic_cdr_stream::read_header()
{
  return entity_properties();
}

bool basic_cdr_stream::structure_is_list(extensibility ext) const
{
  (void) ext;
  return false;
}

void basic_cdr_stream::push_entity(const entity_properties &props)
{
  write_header_fixed(props);

  m_headers.push(props);
  m_headers.top().offset = position();
}

void basic_cdr_stream::write_header_fixed(const entity_properties &props, size_t N)
{
  (void) props;
  (void) N;
}

void basic_cdr_stream::pop_entity()
{
  assert(m_headers.size());

  m_headers.pop();
}

void basic_cdr_stream::move_header(const entity_properties &props)
{
  (void) props;
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
