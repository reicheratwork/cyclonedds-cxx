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
#include <optional>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//randomize optional
template<typename T>
void randomize_contents(std::optional<T> &opt, const size_t max_str_length, const size_t max_seq_length)
{
  if (rand()%2) {
    if (!opt)
      opt.emplace();
    randomize_contents(opt.value(), max_str_length, max_seq_length);
  } else {
    if (opt)
      opt.reset();
  }
}

} //util
} //cyclonedds
} //eclipse
} //org
