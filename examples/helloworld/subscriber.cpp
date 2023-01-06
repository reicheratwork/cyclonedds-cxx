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

/* Include the C++ DDS API. */
#include "dds/dds.hpp"

/* Include data type and specific traits to be used with the C++ DDS API. */
#include "HelloWorldData.hpp"

using namespace org::eclipse::cyclonedds;

int main() {
    try {
        std::cout << "=== [Subscriber] Create reader." << std::endl;

        /* First, a domain participant is needed.
         * Create one on the default domain. */
        dds::domain::DomainParticipant participant(domain::default_id());

        /* To subscribe to something, a topic is needed. */
        dds::topic::Topic<HelloWorldData::Msg> topic(participant, "HelloWorldData_Msg");

        /* A reader also needs a subscriber. */
        dds::sub::Subscriber subscriber(participant);

        /* Now, the reader can be created to subscribe to a HelloWorld message. */
        dds::sub::DataReader<HelloWorldData::Msg> reader(subscriber, topic);

        /* Poll until a message has been read.
         * It isn't really recommended to do this kind wait in a polling loop.
         * It's done here just to illustrate the easiest way to get data.
         * Please take a look at Listeners and WaitSets for much better
         * solutions, albeit somewhat more elaborate ones. */
        std::cout << "=== [Subscriber] Wait for message." << std::endl;

        while (reader.subscription_matched_status().current_count() == 0)
          std::this_thread::sleep_for(std::chrono::milliseconds(20));

        int32_t nextid = -1, invalid_msg = 0, out_order = 0;
        while (reader.subscription_matched_status().current_count()) {
            /* Try taking samples from the reader. */
            auto samples = reader.take();

            for (const auto & p:samples) {
                const auto& msg = p.data();
                const auto& info = p.info();
                if (info.state().view_state() != dds::sub::status::ViewState::new_view())
                  continue;
                else if (!info.valid())
                  invalid_msg++;
                else if (nextid >= 0 && nextid != msg.userID())
                  out_order++;

                if (nextid)
                  nextid = msg.userID()-1;
                else
                  nextid = msg.userID();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (invalid_msg)
          std::cerr << "=== [Subscriber] Invalid messages received: " << invalid_msg << std::endl;
        if (out_order)
          std::cerr << "=== [Subscriber] Out of order userids received: " << out_order << std::endl;
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
