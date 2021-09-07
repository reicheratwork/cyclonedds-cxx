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

#include <org/eclipse/cyclonedds/core/cdr/entity_properties.hpp>
#include <iostream>
#include <algorithm>
#include <cassert>

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
  std::cout << "d: " << depth;
  for (size_t i = 0; i < depth; i++) std::cout << "  ";
  std::cout << prefix << ": s_id: " << s_id << " m_id: " << m_id << " final: " << (is_last ? "yes" : "no");

  std::cout << " p_ext: ";
  switch(p_ext) {
    case ext_final:
    std::cout << "FINAL";
    break;
    case ext_appendable:
    std::cout << "APPENDABLE";
    break;
    case ext_mutable:
    std::cout << "MUTABLE";
    break;
  }
  std::cout << " e_ext: ";
  switch(e_ext) {
    case ext_final:
    std::cout << "FINAL";
    break;
    case ext_appendable:
    std::cout << "APPENDABLE";
    break;
    case ext_mutable:
    std::cout << "MUTABLE";
    break;
  }
  std::cout << std::endl;
  if (recurse) {
    for (const auto & e:m_members_by_seq) e.print(true, depth+1, "s:member");
    for (const auto & e:m_keys_by_seq) e.print(true, depth+1, "s:key   ");
    for (const auto & e:m_members_by_id) e.print(true, depth+1, "i:member");
    for (const auto & e:m_keys_by_id) e.print(true, depth+1, "i:key   ");
  }
}

void entity_properties::set_member_props(uint32_t sequence_id, uint32_t member_id, bool optional)
{
  s_id = sequence_id;
  m_id = member_id;
  is_optional = optional;
}

void entity_properties::finish(bool at_root)
{
  finish_keys(at_root);
  sort_by_member_id();

  for (auto &e : m_members_by_seq) {
    e.finish(false);
  }

  for (auto &e : m_members_by_id) {
    e.finish(false);
  }

  for (auto &e : m_keys_by_seq) {
    e.finish(false);
  }

  for (auto &e : m_keys_by_id) {
    e.finish(false);
  }
}

void entity_properties::finish_keys(bool at_root)
{
  if (!at_root && m_keys_by_seq.size() < 2) {
    if (m_keys_by_seq.size())
      assert(!m_keys_by_seq.back());
    m_keys_by_seq = m_members_by_seq;
  }

  for (auto & e:m_keys_by_seq) {
    e.must_understand = true;
    e.e_ext = ext_final;
    e.p_ext = ext_final;
  }
}

void entity_properties::sort_by_member_id()
{
  m_members_by_id = sort_proplist(m_members_by_seq);
  m_keys_by_id = sort_proplist(m_keys_by_seq);
}

proplist entity_properties::sort_proplist(
  const proplist &in)
{
  auto out = in;
  out.sort(member_id_comp);

  if (out.size()) {
    auto it2 = out.begin();
    auto it = it2++;

    while (it2 != out.end()) {
      if (it2->m_id == it->m_id && it2->is_last == it->is_last) {
        it->merge(*it2);
        it2 = out.erase(it2);
      } else {
        it = it2++;
      }
    }
  }

  return out;
}

void entity_properties::merge(const entity_properties_t &other)
{
  assert(other.m_id == m_id && other.is_last == is_last);

  m_members_by_seq.insert(m_members_by_seq.end(), other.m_members_by_seq.begin(), other.m_members_by_seq.end());

  m_keys_by_seq.insert(m_keys_by_seq.end(), other.m_keys_by_seq.begin(), other.m_keys_by_seq.end());
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
