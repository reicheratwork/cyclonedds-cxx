/* Copyright 2010, Object Management Group, Inc.
* Copyright 2010, PrismTech, Corp.
* Copyright 2010, Real-Time Innovations, Inc.
* All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef OMG_DDS_SUB_DATA_READER_HPP_
#define OMG_DDS_SUB_DATA_READER_HPP_

#include <dds/sub/detail/DataReader.hpp>

namespace dds {
    namespace sub {

        template < typename T,
           template <typename Q> class DELEGATE = dds::sub::detail::DataReader >
           class DataReader;
    }
}

#include <dds/sub/TDataReader.hpp>


// = Manipulators
namespace dds
{
namespace sub
{
namespace functors
{
typedef dds::sub::functors::detail::MaxSamplesManipulatorFunctor      MaxSamplesManipulatorFunctor;
typedef dds::sub::functors::detail::ContentFilterManipulatorFunctor   ContentFilterManipulatorFunctor;
typedef dds::sub::functors::detail::StateFilterManipulatorFunctor   StateFilterManipulatorFunctor;
typedef dds::sub::functors::detail::InstanceManipulatorFunctor     InstanceManipulatorFunctor;
typedef dds::sub::functors::detail::NextInstanceManipulatorFunctor   NextInstanceManipulatorFunctor;
}
}
}

namespace dds
{
namespace sub
{

/**
 * @brief Conducts a read operation using the provided selector.
 *
 * @param selector The selector to use for the read operation.
 *
 * @return Selector& the result of the read.
 */
template <typename SELECTOR>
SELECTOR& read(SELECTOR& selector);

/**
 * @brief Conducts a take operation using the provided selector.
 *
 * @param selector The selector to use for the take operation.
 *
 * @return Selector& the result of the take.
 */
template <typename SELECTOR>
SELECTOR& take(SELECTOR& selector);

/**
 * @brief Constructs a MaxSamplesManipulatorFunctor object.
 *
 * @param n The maximum number of samples to return using the functor.
 *
 * @return MaxSamplesManipulatorFunctor The functor.
 */
inline dds::sub::functors::MaxSamplesManipulatorFunctor
max_samples(uint32_t n);

/**
 * @brief Constructs a ContentFilterManipulatorFunctor object.
 *
 * @param query The query the samples have to obey to return using the functor.
 *
 * @return ContentFilterManipulatorFunctor The functor.
 */
inline dds::sub::functors::ContentFilterManipulatorFunctor
content(const dds::sub::Query& query);

/**
 * @brief Constructs a StateFilterManipulatorFunctor object.
 *
 * @param s The datastate of the samples returned by the functor.
 *
 * @return StateFilterManipulatorFunctor The functor.
 */
inline dds::sub::functors::StateFilterManipulatorFunctor
state(const dds::sub::status::DataState& s);

/**
 * @brief Constructs a InstanceManipulatorFunctor object.
 *
 * @param h The instance handle of the samples returned by the functor.
 *
 * @return InstanceManipulatorFunctor The functor.
 */
inline dds::sub::functors::InstanceManipulatorFunctor
instance(const dds::core::InstanceHandle& h);

/**
 * @brief Constructs a NextInstanceManipulatorFunctor object.
 *
 * @param h The instance handle of the samples returned by the functor.
 *
 * @return NextInstanceManipulatorFunctor The functor.
 */
inline dds::sub::functors::NextInstanceManipulatorFunctor
next_instance(const dds::core::InstanceHandle& h);

}
}


#endif /* OMG_DDS_SUB_DATA_READER_HPP_ */
