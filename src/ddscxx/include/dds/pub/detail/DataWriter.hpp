#ifndef OMG_DDS_PUB_DETAIL_DATA_WRITER_HPP_
#define OMG_DDS_PUB_DETAIL_DATA_WRITER_HPP_

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

#include <dds/topic/Topic.hpp>
#include <dds/pub/AnyDataWriter.hpp>
#include <dds/topic/detail/Topic.hpp>
#include <org/eclipse/cyclonedds/core/EntityDelegate.hpp>
#include <org/eclipse/cyclonedds/topic/TopicTraits.hpp>
#include <org/eclipse/cyclonedds/core/ScopedLock.hpp>
#include <org/eclipse/cyclonedds/pub/AnyDataWriterDelegate.hpp>
#include <dds/dds.h>

namespace dds {
    namespace pub {

        template <typename T>
        class DataWriterListener;

        namespace detail {
            template <typename T>
            class DataWriter;
        }

        template <typename T, template <typename Q> class DELEGATE>
        class DataWriter;
    }
}



/***************************************************************************
 *
 * dds/pub/detail/DataWriter<> DELEGATE declaration.
 * Implementation can be found in dds/pub/detail/DataWriterImpl.hpp
 *
 ***************************************************************************/
template <typename T>
class dds::pub::detail::DataWriter : public ::org::eclipse::cyclonedds::pub::AnyDataWriterDelegate  {
public:
    /**
     * @brief Typedef for brevity of notation.
     */
    typedef typename ::dds::core::smart_ptr_traits< DataWriter<T> >::ref_type ref_type;
    /**
     * @brief Typedef for brevity of notation.
     */
    typedef typename ::dds::core::smart_ptr_traits< DataWriter<T> >::weak_ref_type weak_ref_type;

    DataWriter(const dds::pub::Publisher& pub,
               const ::dds::topic::Topic<T>& topic,
               const dds::pub::qos::DataWriterQos& qos,
               dds::pub::DataWriterListener<T>* listener,
               const dds::core::status::StatusMask& mask);

    /**
     * @brief Destructor (virtual).
     *
     * Will cleanup any resources associated with this entity.
     */
    virtual ~DataWriter();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    /**
     * @brief Check if a Loan is available to this entity.
     *
     * The loan is available if the shared memory is enabled and all the constraints
     * to enable shared memory are met and the type is fixed loan_sample() can be used if and only if
     * is_loan_supported() returns true, otherwise a dds::core::UnsupportedError will be thrown.
     *
     * @returns loan available or not
     */
    bool is_loan_supported();

    /**
     * @brief Loan a sample from the writer.
     *
     * This function is to be used with write() to publish the loaned sample.
     * The function can only be used if is_loan_supported() is true for the writer.
     *
     * @return T& reference to the loaned sample.
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::UnsupportedError
     *             invoked on a writer not supporting loans.
     */
    T& loan_sample();

    /**
     * @brief Return loaned samples to a reader or writer
     *
     * Used to release sample buffers created loan_sample() operation.
     * Writer-loans are normally released implicitly when writing a loaned sample, but you can
     * cancel a writer-loan prematurely by invoking the return_loan() operation.
     *
     * @param[in] sample the loaned sample whose ownership is to revert to the writer.
     *
     * @throws dds::core::InvalidArgumentError
     *             operation is aborted
     * @throws dds::core::PreconditionNotMetError
     *             sample does not correspond to an outstanding loan
     * @throws dds::core::UnsupportedError
     *             invoked on a writer not supporting loans.
     */
    void return_loan(T& sample);

    /**
     * This operation modifies the value of a data instance.
     *
     * <b>Detailed Description</b><br>
     * This operation modifies the value of a data instance. When this operation is used,
     * the Data Distribution Service will automatically supply the value of the
     * source_timestamp that is made available to connected DataReader objects.<br>
     * This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * As a side effect, this operation asserts liveliness on the DataWriter itself and on
     * the containing DomainParticipant.
     *
     * <i>Blocking</i><br>
     * If the dds::core::policy::History QosPolicy is set to KEEP_ALL, the write
     * operation on the DataWriter may block if the modification would cause data to be
     * lost because one of the limits, specified in the dds::core::policy::ResourceLimits, is
     * exceeded. In case the synchronous attribute value of the
     * dds::core::policy::Reliability is set to TRUE for communicating DataWriters and
     * DataReaders then the DataWriter will wait until all synchronous
     * DataReaders have acknowledged the data. Under these circumstances, the
     * max_blocking_time attribute of the dds::core::policy::Reliability configures the
     * maximum time the write operation may block (either waiting for space to become
     * available or data to be acknowledged). If max_blocking_time elapses before the
     * DataWriter is able to store the modification without exceeding the limits and all
     * expected acknowledgements are received, the write operation will fail and throw
     * TimeoutError.
     *
     * @param sample the raw sample to be written
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void write_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample);

    /**
     * This operation modifies the value of a data instance and provides a value for the
     * source_timestamp explicitly.
     *
     * <b>Detailed Description</b><br>
     * It modifies the values of the given data instances. When this operation is used,
     * the application provides the value for the parameter source_timestamp that is made
     * available to connected DataReader objects.<br>
     * This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * As a side effect, this operation asserts liveliness on the DataWriter itself and on
     * the containing DomainParticipant.
     *
     * <i>Blocking</i><br>
     * This operation can be blocked (see @ref anchor_dds_pub_datawriter_write_blocking "write blocking").
     *
     * @param sample the raw sample to be written
     * @param timestamp the timestamp used for this sample
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void write_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample, const dds::core::Time& timestamp);

    /**
     * This operation requests the Data Distribution Service to mark the instance for
     * deletion.
     *
     * <b>Detailed Description</b><br>
     * This operation requests the Data Distribution Service to mark the instance for
     * deletion. Copies of the instance and its corresponding samples, which are stored in
     * every connected DataReader and, dependent on the QosPolicy settings, also in
     * the Transient and Persistent stores, will be marked for deletion by setting their
     * dds::sub::status::InstanceState to not_alive_disposed state.
     *
     * When this operation is used, the Data Distribution Service will automatically supply
     * the value of the source_timestamp that is made available to connected
     * DataReader objects. This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * As a side effect, this operation asserts liveliness on the DataWriter itself and on
     * the containing DomainParticipant.
     *
     * <i>Effects</i><br>
     * This operation @ref anchor_dds_pub_datawriter_dispose_effect_readers "effects DataReaders"
     * and @ref anchor_dds_pub_datawriter_dispose_effect_stores "effects Transient/Persistent Stores".
     *
     * <i>Instance</i><br>
     * The instance is identified by the key fields of the given typed data sample, instead
     * of an InstanceHandle.
     *
     * <i>Blocking</i><br>
     * This operation can be blocked (see @ref anchor_dds_pub_datawriter_dispose_blocking "dispose blocking").
     *
     * @param sample raw sample of the instance to dispose
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::PreconditionNotMetError
     *                  The handle has not been registered with this DataWriter.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void dispose_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample);

    /**
     * This operation requests the Data Distribution Service to mark the instance for
     * deletion and provides a value for the source_timestamp explicitly.
     *
     * <b>Detailed Description</b><br>
     * This operation requests the Data Distribution Service to mark the instance for
     * deletion. Copies of the instance and its corresponding samples, which are stored in
     * every connected DataReader and, dependent on the QosPolicy settings, also in
     * the Transient and Persistent stores, will be marked for deletion by setting their
     * dds::sub::status::InstanceState to not_alive_disposed state.
     *
     * When this operation is used, the application explicitly supplies
     * the value of the source_timestamp that is made available to connected
     * DataReader objects. This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * As a side effect, this operation asserts liveliness on the DataWriter itself and on
     * the containing DomainParticipant.
     *
     * <i>Effects</i><br>
     * This operation @ref anchor_dds_pub_datawriter_dispose_effect_readers "effects DataReaders"
     * and @ref anchor_dds_pub_datawriter_dispose_effect_stores "effects Transient/Persistent Stores".
     *
     * <i>Instance</i><br>
     * The instance is identified by the key fields of the given typed data sample, instead
     * of an InstanceHandle.
     *
     * <i>Blocking</i><br>
     * This operation can be blocked (see @ref anchor_dds_pub_datawriter_dispose_blocking "dispose blocking").
     *
     * @param sample raw sample of the instance to dispose
     * @param timestamp the timestamp
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::PreconditionNotMetError
     *                  The handle has not been registered with this DataWriter.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void dispose_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample, const dds::core::Time& timestamp);

    /**
     * This operation informs the Data Distribution Service that the application will not be
     * modifying a particular instance any more.
     *
     * <b>Detailed Description</b><br>
     * This operation informs the Data Distribution Service that the application will not be
     * modifying a particular instance any more. Therefore, this operation reverses the
     * action of @ref dds::pub::DataWriter::register_instance(const T& key) "\"register_instance\"" or
     * @ref dds::pub::DataWriter::register_instance(const T& key, const dds::core::Time& timestamp) "\"register_instance_w_timestamp\"".
     * register_instance or register_instance_w_timestamp.<br>
     * It should only be called on an instance that is currently registered. This operation
     * should be called just once per instance, regardless of how many times
     * @ref dds::pub::DataWriter::register_instance(const T& key) "register_instance" was called
     * for that instance.<br>
     * This operation also indicates
     * that the Data Distribution Service can locally remove all information regarding that
     * instance. The application should not attempt to use the handle, previously
     * allocated to that instance, after calling this operation.
     *
     * When this operation is used, the Data Distribution Service will automatically supply
     * the value of the source_timestamp that is made available to connected
     * DataReader objects. This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * <i>Effects</i><br>
     * If, after unregistering, the application wants to modify (write or dispose) the
     * instance, it has to register the instance again, or it has to use the default
     * instance handle (InstanceHandle.is_nil() == true).
     * This operation does not indicate that the instance should be deleted (that is the
     * purpose of the @ref dds::pub::DataWriter::dispose_instance(const T& key) "dispose".
     * This operation just indicates that the DataWriter no longer
     * has “anything to say” about the instance. If there is no other DataWriter that
     * has registered the instance as well, then the dds::sub::status::InstanceState in all
     * connected DataReaders will be changed to not_alive_no_writers, provided this
     * InstanceState was not already set to not_alive_disposed. In the last case the
     * InstanceState will not be effected by the unregister_instance call,
     * see also @ref DCPS_Modules_Subscription_SampleInfo "Sample info concept".
     *
     * This operation can affect the ownership of the data instance. If the
     * DataWriter was the exclusive owner of the instance, calling this operation will
     * release that ownership, meaning ownership may be transferred to another,
     * possibly lower strength, DataWriter.
     *
     * The operation must be called only on registered instances. Otherwise the operation
     * trow PreconditionNotMetError.
     *
     * <i>Blocking</i><br>
     * If the dds::core::policy::History QosPolicy is set to KEEP_ALL, the unregister_instance
     * operation on the DataWriter may block if the modification would cause data to be
     * lost because one of the limits, specified in the dds::core::policy::ResourceLimits, is
     * exceeded. In case the synchronous attribute value of the
     * dds::core::policy::Reliability is set to TRUE for communicating DataWriters and
     * DataReaders then the DataWriter will wait until all synchronous
     * DataReaders have acknowledged the data. Under these circumstances, the
     * max_blocking_time attribute of the dds::core::policy::Reliability configures the
     * maximum time the unregister operation may block (either waiting for space to become
     * available or data to be acknowledged). If max_blocking_time elapses before the
     * DataWriter is able to store the modification without exceeding the limits and all
     * expected acknowledgements are received, the unregister_instance operation will fail
     * and throw TimeoutError.
     *
     * @param sample raw sample of the instance to unregister
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::PreconditionNotMetError
     *                  The handle has not been registered with this DataWriter.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void unregister_instance_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample);

    /**
     * This operation will inform the Data Distribution Service that the application will not
     * be modifying a particular instance any more and provides a value for the
     * source_timestamp explicitly.
     *
     * <b>Detailed Description</b><br>
     * This operation informs the Data Distribution Service that the application will not be
     * modifying a particular instance any more. Therefore, this operation reverses the
     * action of @ref dds::pub::DataWriter::register_instance(const T& key) "\"register_instance\"" or
     * @ref dds::pub::DataWriter::register_instance(const T& key, const dds::core::Time& timestamp) "\"register_instance_w_timestamp\"".
     * register_instance or register_instance_w_timestamp.<br>
     * It should only be called on an instance that is currently registered. This operation
     * should be called just once per instance, regardless of how many times
     * @ref dds::pub::DataWriter::register_instance(const T& key) "register_instance" was called
     * for that instance.<br>
     * This operation also indicates
     * that the Data Distribution Service can locally remove all information regarding that
     * instance. The application should not attempt to use the handle, previously
     * allocated to that instance, after calling this operation.
     *
     * When this operation is used, the application itself supplied
     * the value of the source_timestamp that is made available to connected
     * DataReader objects. This timestamp is important for the interpretation of the
     * dds::core::policy::DestinationOrder QosPolicy.
     *
     * <i>Effects</i><br>
     * See @ref anchor_dds_pub_datawriter_unregister_effects "here" for the unregister effects.
     *
     * <i>Blocking</i><br>
     * This operation can be blocked (see @ref anchor_dds_pub_datawriter_unregister_blocking "unregister blocking").
     *
     * @param i the instance to unregister
     * @param timestamp the timestamp
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::NotEnabledError
     *                  The DataWriter has not yet been enabled.
     * @throws dds::core::PreconditionNotMetError
     *                  The handle has not been registered with this DataWriter.
     * @throws dds::core::TimeoutError
     *                  Either the current action overflowed the available resources
     *                  as specified by the combination of the Reliability QosPolicy,
     *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
     *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
     *                  This caused blocking of the write operation, which could not be resolved before
     *                  max_blocking_time of the Reliability QosPolicy elapsed.
     */
    void unregister_instance_cdr(const org::eclipse::cyclonedds::topic::CDRBlob& sample, const dds::core::Time& timestamp);

    void write(const T& sample);

    void write(const T& sample, const dds::core::Time& timestamp);

    void write(const T& sample, const ::dds::core::InstanceHandle& instance);

    void write(const T& sample,
               const ::dds::core::InstanceHandle& instance,
               const dds::core::Time& timestamp);

    void write(const dds::topic::TopicInstance<T>& i);

    void write(const dds::topic::TopicInstance<T>& i,
               const dds::core::Time& timestamp);

    /**
    * @brief This operation modifies and disposes a data instance.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  sample   The data to be written and disposed.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const T& sample);

    /**
    * @brief This operation modifies and disposes a data instance with a specific timestamp.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  sample   The data to be written and disposed.
    * @param[in]  timestamp The timestamp used as source timestamp.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const T& sample, const dds::core::Time& timestamp);

    /**
    * @brief This operation modifies and disposes a specific instance.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  sample   The data to be written to the specified instance handle.
    * @param[in]  instance The instance to be modified and disposed.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const T& sample, const ::dds::core::InstanceHandle& instance);

    /**
    * @brief This operation modifies and disposes a data instance with a specific timestamp.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  sample   The data to be written to the specified instance handle.
    * @param[in]  instance The instance to be modified and disposed.
    * @param[in]  timestamp The timestamp used as source timestamp.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const T& sample,
            const ::dds::core::InstanceHandle& instance,
            const dds::core::Time& timestamp);

    /**
    * @brief This operation modifies and disposes a specific instance.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  i The instance to be modified and disposed.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const dds::topic::TopicInstance<T>& i);


    /**
    * @brief This operation modifies and disposes a data instance with a specific timestamp.
    *
    * This operation requests the Data Distribution Service to modify the instance and
    * mark it for deletion. Copies of the instance and its corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in]  i The instance to be modified and disposed.
    * @param[in]  timestamp The timestamp used as source timestamp.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    void writedispose(const dds::topic::TopicInstance<T>& i,
                      const dds::core::Time& timestamp);

    /**
    * @brief This operation modifies and disposes a range of samples.
    *
    * This operation requests the Data Distribution Service to modify the instances and
    * mark them for deletion. Copies of the instances and their corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in] begin The start of the range of samples to dispose.
    * @param[in] end The end of the range of samples to dispose.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    template <typename FWIterator>
    void writedispose(const FWIterator& begin, const FWIterator& end);


    /**
    * @brief This operation modifies and disposes a range of samples at a specific timestamp.
    *
    * This operation requests the Data Distribution Service to modify the instances and
    * mark them for deletion. Copies of the instances and their corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in] begin The start of the range of samples to dispose.
    * @param[in] end The end of the range of samples to dispose.
    * @param[in] timestamp The timestamp used as source timestamp.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    template <typename FWIterator>
    void writedispose(const FWIterator& begin, const FWIterator& end,
                      const dds::core::Time& timestamp);

    /**
    * @brief This operation modifies and disposes a range of samples and instances.
    *
    * This operation requests the Data Distribution Service to modify the instances and
    * mark them for deletion. Copies of the instances and their corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in] data_begin The start of the range of samples to write to the instances.
    * @param[in] data_end The end of the range of samples to write to the instances.
    * @param[in] handle_begin The start of the range of instances to modify.
    * @param[in] handle_end The end of the range of instances to modify.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void writedispose(const SamplesFWIterator& data_begin,
                      const SamplesFWIterator& data_end,
                      const HandlesFWIterator& handle_begin,
                      const HandlesFWIterator& handle_end);


    /**
    * @brief This operation modifies and disposes a range of samples and instances at a specific timestamp.
    *
    * This operation requests the Data Distribution Service to modify the instances and
    * mark them for deletion. Copies of the instances and their corresponding samples, which are
    * stored in every connected reader and, dependent on the QoS policy settings (also in
    * the Transient and Persistent stores) will be modified and marked for deletion by
    * setting their instance state to DDS_IST_NOT_ALIVE_DISPOSED.
    *
    * If the history QoS policy is set to DDS_HISTORY_KEEP_ALL, this
    * operation on the writer may block if the modification
    * would cause data to be lost because one of the limits, specified in the
    * resource_limits QoS policy, to be exceeded. In case the synchronous
    * attribute value of the reliability Qos policy is set to true for
    * communicating writers and readers then the writer will wait until
    * all synchronous readers have acknowledged the data. Under these
    * circumstances, the max_blocking_time attribute of the reliability
    * QoS policy configures the maximum time the dds_writedispose operation
    * may block.
    * If max_blocking_time elapses before the writer is able to store the
    * modification without exceeding the limits and all expected acknowledgements
    * are received, this operation will fail will throw a TimeoutError.
    *
    * @param[in] data_begin The start of the range of samples to write to the instances.
    * @param[in] data_end The end of the range of samples to write to the instances.
    * @param[in] handle_begin The start of the range of instances to modify.
    * @param[in] handle_end The end of the range of instances to modify.
    * @param[in] timestamp The timestamp used as source timestamp.
    *
    * @throws dds::core::Error
    *                  An internal error has occurred.
    * @throws dds::core::AlreadyClosedError
    *                  The entity has already been closed.
    * @throws dds::core::NotEnabledError
    *                  The DataWriter has not yet been enabled.
    * @throws dds::core::PreconditionNotMetError
    *                  The handle has not been registered with this DataWriter.
    * @throws dds::core::TimeoutError
    *                  Either the current action overflowed the available resources
    *                  as specified by the combination of the Reliability QosPolicy,
    *                  History QosPolicy and ResourceLimits QosPolicy, or the current action
    *                  was waiting for data delivery acknowledgement by synchronous DataReaders.
    *                  This caused blocking of the write operation, which could not be resolved before
    *                  max_blocking_time of the Reliability QosPolicy elapsed.
    * @retval dds::core::IllegalOperationError
    *             The operation is invoked on an inappropriate object.
    */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void writedispose(const SamplesFWIterator& data_begin,
                      const SamplesFWIterator& data_end,
                      const HandlesFWIterator& handle_begin,
                      const HandlesFWIterator& handle_end,
                      const dds::core::Time& timestamp);

    const ::dds::core::InstanceHandle register_instance(const T& key,
                                                        const dds::core::Time& timestamp);

    void unregister_instance(const ::dds::core::InstanceHandle& handle,
                             const dds::core::Time& timestamp);

    void unregister_instance(const T& sample,
                             const dds::core::Time& timestamp);

    void dispose_instance(const ::dds::core::InstanceHandle& handle,
                          const dds::core::Time& timestamp);

    void dispose_instance(const T& sample,
                          const dds::core::Time& timestamp);

    dds::topic::TopicInstance<T>& key_value(dds::topic::TopicInstance<T>& i,
                                            const ::dds::core::InstanceHandle& h);

    T& key_value(T& sample, const ::dds::core::InstanceHandle& h);

    dds::core::InstanceHandle lookup_instance(const T& key);

    const dds::topic::Topic<T>& topic() const;

    virtual const dds::pub::Publisher& publisher() const;

    void listener(DataWriterListener<T>* listener,
                  const ::dds::core::status::StatusMask& mask);

    DataWriterListener<T>* listener() const;

    /**
     * @brief This function closes the entity and releases related resources.
     *
     * Resource management for some reference types might involve relatively heavyweight
     * operating- system resources — such as e.g., threads, mutexes, and network sockets —
     * in addition to memory. These objects therefore provide a method close() that shall
     * halt network communication (in the case of entities) and dispose of any appropriate
     * operating-system resources.
     *
     * Users of this PSM are recommended to call close on objects of all reference types once
     * they are finished using them. In addition, implementations may automatically close
     * objects that they deem to be no longer in use, subject to the following restrictions:
     * - Any object to which the application has a direct reference is still in use.
     * - Any object that has been explicitly retained is still in use
     * - The creator of any object that is still in use is itself still in use.
     */
    virtual void close();

    /**
     * @brief Generates the public facing interface from the private implementation.
     *
     * @return dds::pub::DataWriter<T, dds::pub::detail::DataWriter> the public facing implementation.
     */
    dds::pub::DataWriter<T, dds::pub::detail::DataWriter> wrapper();

    /**
     * @brief Callback function for missed deadlines.
     *
     * Called from CycloneDDS-C when a writer does not adhere to the limits set through the Deadline
     * QoSPolicy. Will call the on_offered_deadline_missed function on this entity's associated
     * listener.
     *
     * @param[in,out] sd delegate for the status to be passed to the listener.
     */
    void on_offered_deadline_missed(dds_entity_t,
          org::eclipse::cyclonedds::core::OfferedDeadlineMissedStatusDelegate &sd);

    /**
     * @brief Callback function for incompatible QoSes.
     *
     * Called from CycloneDDS-C when a reader has attempted to match with this writer, but this failed
     * due to their QoSes not being compatible. Will call the on_offered_incompatible_qos function on
     * this entity's associated listener.
     *
     * @param[in,out] sd delegate for the status to be passed to the listener.
     */
    void on_offered_incompatible_qos(dds_entity_t,
          org::eclipse::cyclonedds::core::OfferedIncompatibleQosStatusDelegate &sd);

    /**
     * @brief Callback function for losing liveliness.
     *
     * Called from CycloneDDS-C when an instance on this writer has not adhered to the limits set by the
     * Liveliness QoSPolicy. Will call the on_liveliness_lost function on this entity's associated listener.
     *
     * @param[in,out] sd delegate for the status to be passed to the listener.
     */
    void on_liveliness_lost(dds_entity_t,
          org::eclipse::cyclonedds::core::LivelinessLostStatusDelegate &sd);

    /**
     * @brief Callback function for matching with readers.
     *
     * Called from CycloneDDS-C when this writer has succesfully matched with a reader. Will call the
     * on_publication_matched function on this entity's associated listener.
     *
     * @param[in,out] sd delegate for the status to be passed to the listener.
     */
    void on_publication_matched(dds_entity_t,
          org::eclipse::cyclonedds::core::PublicationMatchedStatusDelegate &sd);

private:
   dds::pub::Publisher                    pub_;
   dds::topic::Topic<T>                   topic_;
};




#endif /* OMG_DDS_PUB_DETAIL_DATA_WRITER_HPP_ */
