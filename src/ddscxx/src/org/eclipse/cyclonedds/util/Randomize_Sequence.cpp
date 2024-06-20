// Copyright(c) 2006 to 2020 ZettaScale Technology and others
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License v. 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
// v. 1.0 which is available at
// http://www.eclipse.org/org/documents/edl-v10.php.
//
// SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause

#include <org/eclipse/cyclonedds/util/Randomize_Sequence.hpp>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

template<>
void randomize_contents<std::vector<bool, std::allocator<bool>>>(std::vector<bool, std::allocator<bool>> &vec, const size_t, const size_t max_seq_length)
{

  //randomize length
  size_t len = static_cast<size_t>(rand())%max_seq_length;
  vec.resize(len);

  
  //randomize contents
  for (auto e:vec) {
    e = static_cast<bool>(rand()%2);
  }
}

} //util
} //cyclonedds
} //eclipse
} //org
