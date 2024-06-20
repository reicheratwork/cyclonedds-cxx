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
#include <string>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace util {

//randomize string
template<>
void randomize_contents<std::string>(std::string &str, const size_t max_str_length, const size_t);

} //util
} //cyclonedds
} //eclipse
} //org
