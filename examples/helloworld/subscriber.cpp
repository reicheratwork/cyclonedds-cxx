/*
 * Copyright(c) 2006 to 2021 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#include "HelloWorldData.hpp"
#include "Traits_Approach.hpp"

int main() {
    try {
        std::cout << "=== [Subscriber] Create reader." << std::endl;

        dds::domain::DomainParticipant participant(org::eclipse::cyclonedds::domain::default_id());

        example::container<HelloWorldData::Msg>::create(participant, "HelloWorldData_Msg");

        auto &reader = example::container<HelloWorldData::Msg>::get_reader();

        std::cout << "=== [Subscriber] Wait for message." << std::endl;
        bool poll = true;
        while (poll) {
            for (const auto & sample:reader.take()) {
                if (sample.info().valid()) {
                    auto &msg = sample.data();
                    std::cout << "=== [Subscriber] Message received:" << std::endl;
                    std::cout << "    userID  : " << msg.userID() << std::endl;
                    std::cout << "    Message : \"" << msg.message() << "\"" << std::endl;

                    poll = false;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (const dds::core::Exception& e) {
        std::cerr << "=== [Subscriber] DDS exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "=== [Subscriber] C++ exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "=== [Subscriber] Done." << std::endl;

    return EXIT_SUCCESS;
}
