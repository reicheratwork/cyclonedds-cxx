#ifndef OMG_TDDS_CORE_STATUS_STATUS_HPP_
#define OMG_TDDS_CORE_STATUS_STATUS_HPP_

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

#include <dds/core/Value.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/core/policy/QosPolicyCount.hpp>
#include <dds/core/status/State.hpp>

namespace dds
{
namespace core
{
namespace status
{

/**
 * @brief Inconsistent topic status.
 *
 * Records the number of times the status "InconsistentTopic" was triggered.
 * This occurs when an existing Topic observes the (attempted) creation of another
 * with the same name as its own, but with an incompatible datatype.
 */
template <typename D>
class TInconsistentTopicStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TInconsistentTopicStatus();

public:
    /**
     * @return Total cumulative count of all inconsistent topics detected.
     */
    int32_t total_count() const;

    /**
     * @return The incremental number of inconsistent topics since the last time
     * the listener was called or the status was read.
     */
    int32_t total_count_change() const;
};

/**
 * @brief Sample lost status.
 *
 * Records the number of times a DataReader determined that a sample that was written
 * to the topic it is on, was not received by it.
 */
template <typename D>
class TSampleLostStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TSampleLostStatus();

public:
    /**
     * @return Total cumulative count of all samples lost across of instances of data
     * published under the Topic.
     */
    int32_t total_count() const;

    /**
     * @return The incremental number of samples lost since the last time the listener
     * was called or the status was read.
     */
    int32_t total_count_change() const;
};

/**
 * @brief Sample rejected status.
 *
 * Records the number of times a DataReader determined that a sample that was written
 * to the topic it is on, was rejected by it.
 */
template <typename D>
class TSampleRejectedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TSampleRejectedStatus();

public:
    /**
     * @return Total cumulative count of samples rejected by the DataReader.
     */
    int32_t total_count() const;

    /**
     * @return The incremental number of samples rejected since the last time the
     * listener was called or the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return Reason for rejecting the last sample rejected. If no samples have been
     * rejected, the reason is the special value NOT_REJECTED.
     */
    const dds::core::status::SampleRejectedState last_reason() const;

    /**
     * @return Handle to the instance being updated by the last sample that was
     * rejected.
     */
    const dds::core::InstanceHandle last_instance_handle() const;
};

/**
 * @brief Liveliness lost status.
 *
 * Records the number of times the status of an instance has gone from "alive" to
 * "not alive".
 */
template <typename D>
class TLivelinessLostStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TLivelinessLostStatus();

public:
    /**
     * @return Total cumulative number of times that a previously-alive DataWriter
     * became 'not alive' due to a failure to actively signal its liveliness within
     * its offered liveliness period. This count does not change when an
     * already not alive DataWriter simply remains not alive for another
     * liveliness period.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;
};

/**
 * @brief Liveliness changed status.
 *
 * Records the number of times a DataWriter determined that a sample's alive status has changed.
 */
template <typename D>
class TLivelinessChangedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TLivelinessChangedStatus();

public:
    /**
     * @return The total number of currently active DataWriters that write the Topic
     * read by the DataReader. This count increases when a newly-matched
     * DataWriter asserts its liveliness for the first time or when a DataWriter
     * previously considered to be not alive reasserts its liveliness. The count
     * decreases when a DataWriter considered alive fails to assert its
     * liveliness and becomes not alive, whether because it was deleted
     * normally or for some other reason.
     */
    int32_t alive_count() const;

    /**
     * @return The total count of currently DataWriters that write the Topic read by
     * the DataReader that are no longer asserting their liveliness. This count
     * increases when a DataWriter considered alive fails to assert its
     * liveliness and becomes not alive for some reason other than the normal
     * deletion of that DataWriter. It decreases when a previously not alive
     * DataWriter either reasserts its liveliness or is deleted normally.
     */
    int32_t not_alive_count() const;

    /**
     * @return The change in the alive_count since the last time the listener was
     * called or the status was read.
     */
    int32_t alive_count_change() const;

    /**
     * @return The change in the not_alive_count since the last time the listener was
     * called or the status was read.
     */
    int32_t not_alive_count_change() const;

    /**
     * @return Handle to the last DataWriter whose change in liveliness caused this
     * status to change.
     */
    const dds::core::InstanceHandle last_publication_handle() const;
};

/**
 * @brief Offered deadline missed status.
 *
 * Records the number of times a deadline promised by a DataWriter through the Deadline QoSPolicy was
 * not adhered to.
 */
template <typename D>
class TOfferedDeadlineMissedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TOfferedDeadlineMissedStatus();

public:
    /**
     * @return Total cumulative number of offered deadline periods elapsed during
     * which a DataWriter failed to provide data. Missed deadlines
     * accumulate; that is, each deadline period the total_count will be
     * incremented by one.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return Handle to the last instance in the DataWriter for which an offered
     * deadline was missed.
     */
    const dds::core::InstanceHandle last_instance_handle() const;
};

/**
 * @brief Requested deadline missed status.
 *
 * Records the number of times a deadline requested by a DataReader through the Deadline QoSPolicy was
 * not adhered to.
 */
template <typename D>
class TRequestedDeadlineMissedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TRequestedDeadlineMissedStatus();
public:

    /**
     * @return Total cumulative number of missed deadlines detected for any instance
     * read by the DataReader. Missed deadlines accumulate; that is, each
     * deadline period the total_count will be incremented by one for each
     * instance for which data was not received.
     */
    int32_t total_count() const;

    /**
     * @return The incremental number of deadlines detected since the last time the
     * listener was called or the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return Handle to the last instance in the DataReader for which a deadline was
     * detected.
     */
    const dds::core::InstanceHandle last_instance_handle() const;
};

/**
 * @brief Offered incompatible QoS status.
 *
 * Records the number of times the DataWriter encountered a DataReader it attempted to match
 * with, but couldn't, due to their QoSes not being compatible.
 */
template <typename D>
class TOfferedIncompatibleQosStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TOfferedIncompatibleQosStatus();

public:
    /**
     * @return Total cumulative number of times the concerned DataWriter
     * discovered a DataReader for the same Topic with a requested QoS that
     * is incompatible with that offered by the DataWriter.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return The PolicyId of one of the policies that was found to be
     * incompatible the last time an incompatibility was detected.
     */
    dds::core::policy::QosPolicyId last_policy_id() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataWriter discovered a DataReader for the same Topic
     * with a requested QoS that is incompatible with that offered by the
     * DataWriter.
     */
    const dds::core::policy::QosPolicyCountSeq policies() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataWriter discovered a DataReader for the same Topic
     * with a requested QoS that is incompatible with that offered by the
     * DataWriter.
     *
     * @param dst The destination QosPolicyCountSeq the policies will be returned to
     */
    const dds::core::policy::QosPolicyCountSeq&
    policies(dds::core::policy::QosPolicyCountSeq& dst) const;
};

/**
 * @brief Requested incompatible QoS status.
 *
 * Records the number of times the DataReader encountered a DataWriter it attempted to match
 * with, but couldn't, due to their QoSes not being compatible.
 */
template <typename D>
class TRequestedIncompatibleQosStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TRequestedIncompatibleQosStatus();

public:
    /**
     * @return Total cumulative number of times the concerned DataReader
     * discovered a DataWriter for the same Topic with an offered QoS that
     * was incompatible with that requested by the DataReader.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return The QosPolicyId of one of the policies that was found to be
     * incompatible the last time an incompatibility was detected.
     */
    dds::core::policy::QosPolicyId last_policy_id() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataReader discovered a DataWriter for the same Topic
     * with an offered QoS that is incompatible with that requested by the
     * DataReader.
     */
    const dds::core::policy::QosPolicyCountSeq policies() const;

    /**
     * @return A list containing for each policy the total number of times that the
     * concerned DataReader discovered a DataWriter for the same Topic
     * with an offered QoS that is incompatible with that requested by the
     * DataReader.
     *
     * @param dst The destination QosPolicyCountSeq the policies will be returned to
     */
    const dds::core::policy::QosPolicyCountSeq&
    policies(dds::core::policy::QosPolicyCountSeq& dst) const;
};

/**
 * @brief Publication matched status.
 *
 * Records the number of times the DataWriter has matched with a DataReader.
 */
template <typename D>
class TPublicationMatchedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TPublicationMatchedStatus();

public:
    /**
     * @return Total cumulative count the concerned DataWriter discovered a
     * "match" with a DataReader. That is, it found a DataReader for the
     * same Topic with a requested QoS that is compatible with that offered
     * by the DataWriter.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return The number of DataReaders currently matched to the concerned
     * DataWriter.
     */
    int32_t current_count() const;

    /**
     * @return The change in current_count since the last time the listener was called
     * or the status was read.
     */
    int32_t current_count_change() const;

    /**
     * @return Handle to the last DataReader that matched the DataWriter causing the
     * status to change.
     */
    const dds::core::InstanceHandle last_subscription_handle() const;
};

/**
 * @brief Subscription matched status.
 *
 * Records the number of times the DataReader has matched with a DataWriter.
 */
template <typename D>
class TSubscriptionMatchedStatus : public dds::core::Value<D>
{
public:
    /**
     * @brief Default constructor.
     */
    TSubscriptionMatchedStatus();

public:
    /**
     * @return Total cumulative count the concerned DataReader discovered a
     * "match" with a DataWriter. That is, it found a DataWriter for the same
     * Topic with a requested QoS that is compatible with that offered by the
     * DataReader.
     */
    int32_t total_count() const;

    /**
     * @return The change in total_count since the last time the listener was called or
     * the status was read.
     */
    int32_t total_count_change() const;

    /**
     * @return The number of DataWriters currently matched to the concerned
     * DataReader.
     */
    int32_t current_count() const;

    /**
     * @return The change in current_count since the last time the listener was called
     * or the status was read.
     */
    int32_t current_count_change() const;

    /**
     * @return Handle to the last DataWriter that matched the DataReader causing the
     * status to change.
     */
    const dds::core::InstanceHandle last_publication_handle() const;
};

}
}
}/* namespace dds::core::status */

#endif /* OMG_TDDS_CORE_STATUS_STATUS_HPP_ */
