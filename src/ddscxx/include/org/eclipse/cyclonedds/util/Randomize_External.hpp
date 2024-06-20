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
#include <dds/core/External.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//randomize external
template<typename T>
void randomize_contents(dds::core::external<T> &ext, const size_t max_str_length, const size_t max_seq_length)
{
  if (rand()%2) {
    ext = dds::core::external<T>(new T(), true);
    randomize_contents(*ext, max_str_length, max_seq_length);
  } else {
    ext = dds::core::external<T>();
  }
}

} //util
} //cyclonedds
} //eclipse
} //org
