#
# Copyright(c) 2006 to 2019 ADLINK Technology Limited and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#

idlcxx_generate(TARGET echomsg FILES echomsg.idl)

add_library(echo echo.cpp)

target_link_libraries(echo CycloneDDS-CXX::ddscxx echomsg)

add_executable(self_contained self_contained.cpp)
add_executable(sender sender.cpp)
add_executable(answerer answerer.cpp)
target_link_libraries(self_contained echo pthread)
target_link_libraries(sender echo)
target_link_libraries(answerer echo)
set_property(TARGET self_contained PROPERTY CXX_STANDARD 11)
set_property(TARGET sender PROPERTY CXX_STANDARD 11)
set_property(TARGET answerer PROPERTY CXX_STANDARD 11)
