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
#include <array>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//randomize array
template<typename T, size_t N>
void randomize_contents(std::array<T, N> &arr, const size_t max_str_length, const size_t max_seq_length)
{
  //randomize contents
  for (auto & e:arr) {
    randomize_contents(e, max_str_length, max_seq_length);
  }
}

} //util
} //cyclonedds
} //eclipse
} //org
