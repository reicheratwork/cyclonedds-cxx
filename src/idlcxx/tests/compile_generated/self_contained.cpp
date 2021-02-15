/*
 * Copyright(c) 2006 to 2021 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include <thread>

#include "echo.hpp"

int main() {
  std::thread
    send_thread(cxx_test_echofunctions::sender, "ping_topic", "pong_topic", 256),
    answer_thread(cxx_test_echofunctions::answerer, "pong_topic", "ping_topic");

  send_thread.join();
  answer_thread.join();

  return 0;
}
