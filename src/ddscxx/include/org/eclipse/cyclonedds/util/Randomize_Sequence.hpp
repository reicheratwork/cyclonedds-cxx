// Copyright(c) 2024 ZettaScale Technology and others
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License v. 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
// v. 1.0 which is available at
// http://www.eclipse.org/org/documents/edl-v10.php.
//
// SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 
/**
 * @file
 */

#pragma once

#include <org/eclipse/cyclonedds/util/Randomize.hpp>
#include <vector>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//forward declaration
template<typename T, std::enable_if_t<!std::is_same<T,bool>::value, bool> = true >
void randomize_contents(std::vector<T, std::allocator<T>> &vec, const size_t max_str_length, const size_t max_seq_length);

//randomize sequence
template<typename T, std::enable_if_t<!std::is_same<T,bool>::value, bool> = true >
void randomize_contents(std::vector<T, std::allocator<T>> &vec, const size_t max_str_length, const size_t max_seq_length)
{

  //randomize length
  size_t len = static_cast<size_t>(rand())%max_seq_length;
  vec.resize(len);

  
  //randomize contents
  for (auto & e:vec) {
    randomize_contents(e, max_str_length, max_seq_length);
  }
}

//explicit specialization of bool vector
template<>
void randomize_contents(std::vector<bool, std::allocator<bool>> &vec, const size_t max_str_length, const size_t max_seq_length);

} //util
} //cyclonedds
} //eclipse
} //org
