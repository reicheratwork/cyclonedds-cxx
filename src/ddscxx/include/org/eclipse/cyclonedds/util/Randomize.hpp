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

#include <type_traits>
#include <cstddef>
#include <stdlib.h>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//fallback
template<typename T, std::enable_if_t<!std::is_arithmetic<T>::value, bool> = true >
void randomize_contents(T &, const size_t, const size_t);

//randomize integer value
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true >
void randomize_contents(T &entity, const size_t, const size_t)
{
  entity = static_cast<T>(rand());
}

template<typename T, typename D>
D randomize_discriminator();

} //util
} //cyclonedds
} //eclipse
} //org
