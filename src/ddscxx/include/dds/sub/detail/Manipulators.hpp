/*
 * Copyright(c) 2006 to 2020 ZettaScale Technology and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef CYCLONEDDS_DDS_SUB_DETAIL_MANIPULATOR_HPP_
#define CYCLONEDDS_DDS_SUB_DETAIL_MANIPULATOR_HPP_

/**
 * @file
 */

#include <dds/sub/Query.hpp>

namespace dds
{
namespace sub
{
namespace functors
{
namespace detail
{

/**
 * @brief Functor which allows the user to specify the maximum number of samples returned.
 *
 * Used when passed as a Selector function to a read/take operation. The effect is that
 * this operation will not return more than the specified number of samples.
 */
class MaxSamplesManipulatorFunctor
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the functor with the specified maximum number of samples.
     *
     * @param[in] n the maximum number of samples.
     */
    MaxSamplesManipulatorFunctor(uint32_t n) :
        n_(n)
    {
    }

    /**
     * @brief Selector propagator function.
     *
     * Allows the the functor to propagate its the parameters to the Selector S.
     *
     * @param[in] s the selector whose parameters need to be propagated to.
     */
    template<typename S>
    void operator()(S& s)
    {
        s.max_samples(n_);
    }
private:
    uint32_t n_;
};

/**
 * @brief Functor which allows the user to filter the samples returned on their content.
 *
 * Used when passed as a Selector function to a read/take operation. The effect is that
 * only samples which match the supplied query ("dds::sub::Query") are returned in the
 * operation. This query is an "SQL expression" describing the restrictions on the samples'
 * contents.
 */
class ContentFilterManipulatorFunctor
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the functor with the specified query.
     *
     * @param[in] q the query expression.
     */
    ContentFilterManipulatorFunctor(const dds::sub::Query& q) :
        query_(q)
    {
    }

    /**
     * @brief Selector propagator function.
     *
     * Allows the the functor to propagate its the parameters to the Selector S.
     *
     * @param[in] s the selector whose parameters need to be propagated to.
     */
    template<typename S>
    void operator()(S& s)
    {
        s.content(query_);
    }
private:
    const dds::sub::Query query_;
};

/**
 * @brief Functor which allows the user to filter the samples returned on their status.
 *
 * Used when passed as a Selector function to a read/take operation. The effect is that
 * only samples which match the supplied status mask are returned. This mask encompasses
 * the following statuses:
 * - sample_state: the read status of the sample
 * - view_state: the view status of the sample
 * - instance_state: the alive status of the sample
 */
class StateFilterManipulatorFunctor
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the functor with the specified data state mask.
     *
     * @param[in] s the data state mask.
     */
    StateFilterManipulatorFunctor(
        const dds::sub::status::DataState& s) :
        state_(s)
    {
    }

    /**
     * @brief Selector propagator function.
     *
     * Allows the the functor to propagate its the parameters to the Selector S.
     *
     * @param[in] s the selector whose parameters need to be propagated to.
     */
    template<typename S>
    void operator()(S& s)
    {
        s.state(state_);
    }
private:
    dds::sub::status::DataState state_;
};

/**
 * @brief Functor which allows the user to filter the samples returned on specific instance handles.
 *
 * Used when passed as a Selector function to a read/take operation. The effect is that
 * only samples which are associated with a specific instance handle are returned.
 */
class InstanceManipulatorFunctor
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the functor with the specified instance handle.
     *
     * @param[in] h the instance handle.
     */
    InstanceManipulatorFunctor(const dds::core::InstanceHandle& h) :
        handle_(h)
    {
    }

    /**
     * @brief Selector propagator function.
     *
     * Allows the the functor to propagate its the parameters to the Selector S.
     *
     * @param[in] s the selector whose parameters need to be propagated to.
     */
    template<typename S>
    void operator()(S& s)
    {
        s.instance(handle_);
    }
private:
    dds::core::InstanceHandle handle_;
};

/**
 * @brief Functor which allows the user to filter the samples returned on specific instance handles.
 *
 * Used when passed as a Selector function to a read/take operation.
 * !!!TODO!!! HOW IS THIS DIFFERENT FROM InstanceManipulatorFunctor???
 */
class NextInstanceManipulatorFunctor
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the functor with the specified instance handle.
     *
     * @param[in] h the instance handle.
     */
    NextInstanceManipulatorFunctor(
        const dds::core::InstanceHandle& h) :
        handle_(h)
    {
    }

    /**
     * @brief Selector propagator function.
     *
     * Allows the the functor to propagate its the parameters to the Selector S.
     *
     * @param[in] s the selector whose parameters need to be propagated to.
     */
    template<typename S>
    void operator()(S& s)
    {
        s.next_instance(handle_);
    }
private:
    dds::core::InstanceHandle handle_;
};


}
}
}
}



#endif /* CYCLONEDDS_DDS_SUB_DETAIL_MANIPULATOR_HPP_ */
