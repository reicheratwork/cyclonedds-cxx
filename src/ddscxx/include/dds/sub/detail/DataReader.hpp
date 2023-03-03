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
#ifndef OMG_DDS_SUB_DETAIL_DATA_READER_HPP_
#define OMG_DDS_SUB_DETAIL_DATA_READER_HPP_

#include <dds/topic/Topic.hpp>
#include <dds/topic/TopicInstance.hpp>

#include <dds/core/status/Status.hpp>
#include <dds/sub/status/detail/DataStateImpl.hpp>
#include <dds/sub/detail/Manipulators.hpp>
#include <dds/sub/LoanedSamples.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/Query.hpp>

#include <org/eclipse/cyclonedds/core/EntityDelegate.hpp>
#include <org/eclipse/cyclonedds/sub/AnyDataReaderDelegate.hpp>

#include <org/eclipse/cyclonedds/core/ScopedLock.hpp>
#include <org/eclipse/cyclonedds/ForwardDeclarations.hpp>

#include <dds/dds.h>

/***************************************************************************
 *
 * dds/sub/detail/DataReader<> DELEGATE declaration.
 * Implementation can be found in dds/sub/detail/TDataReaderImpl.hpp
 *
 ***************************************************************************/
template <typename T>
class dds::sub::detail::DataReader : public ::org::eclipse::cyclonedds::sub::AnyDataReaderDelegate
{
public:
    /**
     * @brief Typedef for brevity of notation.
     */
    typedef typename ::dds::core::smart_ptr_traits< DataReader<T> >::ref_type ref_type;
    /**
     * @brief Typedef for brevity of notation.
     */
    typedef typename ::dds::core::smart_ptr_traits< DataReader<T> >::weak_ref_type weak_ref_type;

    DataReader(const dds::sub::Subscriber& sub,
               const dds::topic::Topic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

    DataReader(const dds::sub::Subscriber& sub,
               const dds::topic::ContentFilteredTopic<T, dds::topic::detail::ContentFilteredTopic>& topic,
               const dds::sub::qos::DataReaderQos& qos,
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

    /**
     * @brief Common constructor function, performs actions common to all DataReader constructors.
     *
     * Will attempt to create the CycloneDDS-C counterparts of the C++ DataReader.
     *
     * @throw precondition_not_met in the case the DataType is not a Topic
     * @throw other if the creation of the CycloneDDS-C reader failed
     *
     * @param[in] listener listener to be attached to the constructed DataReader
     * @param[in] mask indicates the statuses the listener should monitor
     */
    void common_constructor(dds::sub::DataReaderListener<T>* listener,
                            const dds::core::status::StatusMask& mask);

    /**
     * @brief Destructor.
     *
     * Is virtual, so can be overridden by derived classes.
     */
    virtual ~DataReader();

    /**
     * @brief Function is not yet implemented!
     */
    void copy_samples(
      dds::sub::detail::SamplesHolder& samples,
      void**& c_sample_pointers,
      dds_sample_info_t*& c_sample_infos,
      int num_read);

    /**
     * @brief Object initialization function.
     *
     * Initializes the DataReader with the supplied reference. It then becomes owner of this reference.
     *
     * @param[in] weak_ref the reference to initialize the DataReader with
     */
    void init(ObjectDelegate::weak_ref_type weak_ref);

    /**
     * @brief Default filter state getter function.
     *
     * @return dds::sub::status::DataState the default filter state.
     */
    dds::sub::status::DataState default_filter_state();

    /**
     * @brief Default filter state setter function.
     *
     * @param[in] state the new default filter state to set.
     */
    void default_filter_state(const dds::sub::status::DataState& state);


    /**
     * @brief Loan mechanism support check function.
     *
     * Returns true when the DataReader is able to provide loaned samples.
     *
     * @retval true when loans are supported
     * @retval false when loans are not supported
     */
    bool is_loan_supported();

    /**
     * @brief CDR read function.
     *
     * This will take all samples in serialized format from the reader history cache, set their read status to true and return them.
     * Successive calls to read will return these samples until they are "taken".
     *
     * @return dds::sub::LoanedSamples<org::eclipse::cyclonedds::topic::CDRBlob> the serialized samples from the reader.
     */
    dds::sub::LoanedSamples<org::eclipse::cyclonedds::topic::CDRBlob> read_cdr();

    /**
     * @brief CDR take function.
     *
     * This will take all samples in serialized format from the reader history cache, set their read status to true and return them.
     * Successive calls to "read" or "take" will not return these samples,
     *
     * @return dds::sub::LoanedSamples<org::eclipse::cyclonedds::topic::CDRBlob> the serialized samples from the reader.
     */
    dds::sub::LoanedSamples<org::eclipse::cyclonedds::topic::CDRBlob> take_cdr();

    /**
     * @brief read function.
     *
     * This will take all samples from the reader history cache, set their read status to true and return them.
     * Successive calls to read will return these samples until they are "taken".
     *
     * @return dds::sub::LoanedSamples<T> the samples from the reader.
     */
    dds::sub::LoanedSamples<T> read();

    /**
     * @brief take function.
     *
     * This will take all samples from the reader history cache, set their read status to true and return them.
     * Successive calls to "read" or "take" will not return these samples,
     *
     * @return dds::sub::LoanedSamples<T> the samples from the reader.
     */
    dds::sub::LoanedSamples<T> take();

    template<typename SamplesFWIterator>
    uint32_t read(SamplesFWIterator samples, uint32_t max_samples);
    template<typename SamplesFWIterator>
    uint32_t take(SamplesFWIterator samples, uint32_t max_samples);

    template<typename SamplesBIIterator>
    uint32_t read(SamplesBIIterator samples);
    template<typename SamplesBIIterator>
    uint32_t take(SamplesBIIterator samples);

    /**
     * @brief Get the sample's key value based on a specific instance handle.
     *
     * @param[in] h the handle to retrieve the instance for.
     *
     * @return dds::topic::TopicInstance<T> the instancehandle belonging to the supplied instancehandle.
     */
    dds::topic::TopicInstance<T> key_value(const dds::core::InstanceHandle& h);

    /**
     * @brief Get the sample associated with a specific key value and instance handle.
     *
     * @param[in] key the key value of the instance.
     * @param[in] h the handle to retrieve the instance for.
     *
     * @return T the instance belonging to the supplied keyvalue and instancehandle.
     */
    T& key_value(T& key, const dds::core::InstanceHandle& h);

    /**
     * @brief Get the instance handle for a specific key value.
     *
     * @param[in] key the key sample to retrieve the instancehandle for
     *
     * @return dds::core::InstanceHandle the instancehandle belonging to the supplied key.
     */
    const dds::core::InstanceHandle lookup_instance(const T& key) const;

    /**
     * @brief Subscriber getter function.
     *
     * Returns a reference to the subscriber this reader is subordinated to.
     *
     * @return dds::sub::Subscriber const reference to this DataReader's subscriber.
     */
    virtual const dds::sub::Subscriber& subscriber() const;

    /**
     * @brief DataReader end of life function.
     *
     * Signals that the DataReader will stop existing. This will prevent callbacks being executed,
     * the listeners are decoupled, and this entity is removed from the entity registry.
     */
    void close();

    /**
     * @brief Listener getter function.
     *
     * Returns the listener associated with this DataReader.
     *
     * @return dds::sub::DataReaderListener<T>* the listener for this DataReader.
     */
    dds::sub::DataReaderListener<T>* listener();
    void listener(dds::sub::DataReaderListener<T>* l,
                  const dds::core::status::StatusMask& event_mask);

    /**
     * @brief wrapper function.
     *
     * Produces the public facing implementation from a private instance.
     *
     * @return dds::sub::DataReader<T, dds::sub::detail::DataReader> the public interface class produced from this detail implementation.
     */
    dds::sub::DataReader<T, dds::sub::detail::DataReader> wrapper();

    /**
     * @brief on requested deadline missed callback function.
     *
     * This function will be called when an instance on this reader has not received prompt enough updates
     * as specified through the Deadline QoSPolicy.
     * This will call the on_requested_deadline_missed function on the listener associated with this DataReader.
     */
    void on_requested_deadline_missed(dds_entity_t,
          org::eclipse::cyclonedds::core::RequestedDeadlineMissedStatusDelegate &);

    /**
     * @brief on requested incompatible qos callback function.
     *
     * This function will be called when the QoSPolicies as requested by this DataReader are incompatible with
     * a DataWriter on the same Topic.
     * This will call the on_requested_incompatible_qos function on the listener associated with this DataReader.
     */
    void on_requested_incompatible_qos(dds_entity_t,
          org::eclipse::cyclonedds::core::RequestedIncompatibleQosStatusDelegate &);

    /**
     * @brief on sample rejected callback function.
     *
     * This function will be called when a sample is received, but rejected by this DataReader for whatever reason.
     * This will call the on_sample_rejected function on the listener associated with this DataReader.
     */
    void on_sample_rejected(dds_entity_t,
          org::eclipse::cyclonedds::core::SampleRejectedStatusDelegate &);

    /**
     * @brief on liveliness changed callback function.
     *
     * This function will be called when the liveliness of an instance on this reader changes.
     * This will call the on_liveliness_changed function on the listener associated with this DataReader.
     */
    void on_liveliness_changed(dds_entity_t,
          org::eclipse::cyclonedds::core::LivelinessChangedStatusDelegate &);

    /**
     * @brief on data available callback function.
     *
     * This function will be called when data becomes available on this DataReader.
     * This will call the on_data_available function on the listener associated with this DataReader.
     */
    void on_data_available(dds_entity_t);

    /**
     * @brief on subscription matched callback function.
     *
     * This function will be called when the DataReader matches with a DataWriter.
     * This will call the on_subscription_matched function on the listener associated with this DataReader.
     */
    void on_subscription_matched(dds_entity_t,
          org::eclipse::cyclonedds::core::SubscriptionMatchedStatusDelegate &);

    /**
     * @brief on sample lost callback function.
     *
     * This function will be called when the DataReader determines that it did not receive a sample that was written to the Topic.
     * This will call the on_sample_lost function on the listener associated with this DataReader.
     */
    void on_sample_lost(dds_entity_t,
          org::eclipse::cyclonedds::core::SampleLostStatusDelegate &);

private:
    dds::sub::Subscriber sub_;
    dds::sub::status::DataState status_filter_;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    enum SelectMode {
        SELECT_MODE_READ,
        SELECT_MODE_READ_INSTANCE,
        SELECT_MODE_READ_NEXT_INSTANCE,
        SELECT_MODE_READ_WITH_CONDITION,
        SELECT_MODE_READ_INSTANCE_WITH_CONDITION,
        SELECT_MODE_READ_NEXT_INSTANCE_WITH_CONDITION
    };


public:

    class Selector
    {
    public:
        Selector(typename DataReader<T>::ref_type dr);

        Selector& instance(const dds::core::InstanceHandle& h);
        Selector& next_instance(const dds::core::InstanceHandle& h);
        Selector& filter_state(const dds::sub::status::DataState& s);
        Selector& max_samples(uint32_t n);
        Selector& filter_content(const dds::sub::Query& query);

        dds::sub::LoanedSamples<T> read();
        dds::sub::LoanedSamples<T> take();

        // --- Forward Iterators: --- //
        template<typename SamplesFWIterator>
        uint32_t read(SamplesFWIterator sfit, uint32_t max_samples);
        template<typename SamplesFWIterator>
        uint32_t take(SamplesFWIterator sfit, uint32_t max_samples);

        // --- Back-Inserting Iterators: --- //
        template<typename SamplesBIIterator>
        uint32_t read(SamplesBIIterator sbit);
        template<typename SamplesBIIterator>
        uint32_t take(SamplesBIIterator sbit);

        SelectMode get_mode() const;

    private:
        friend class DataReader;
        SelectMode mode;
        typename DataReader<T>::ref_type reader;
        dds::sub::status::DataState state_filter_;
        bool state_filter_is_set_;
        dds::core::InstanceHandle handle;
        uint32_t max_samples_;
        dds::sub::Query query_;
    };


    class ManipulatorSelector: public Selector
    {
    public:
        //ManipulatorSelector(DataReader<T>* dr);
        ManipulatorSelector(typename DataReader<T>::ref_type dr);

        bool read_mode();
        void read_mode(bool b);

        ManipulatorSelector&
        operator >>(dds::sub::LoanedSamples<T>& samples);

    private:
        bool read_mode_;
    };


private:
    // ==============================================================
    // == Selector Read/Take API

    dds::sub::LoanedSamples<T> read(const Selector& selector);

    dds::sub::LoanedSamples<T> take(const Selector& selector);

    // --- Forward Iterators: --- //
    template<typename SamplesFWIterator>
    uint32_t read(SamplesFWIterator samples,
                  uint32_t max_samples, const Selector& selector);

    template<typename SamplesFWIterator>
    uint32_t take(SamplesFWIterator samples,
                  uint32_t max_samples, const Selector& selector);

    // --- Back-Inserting Iterators: --- //
    template<typename SamplesBIIterator>
    uint32_t read(SamplesBIIterator samples, const Selector& selector);

    template<typename SamplesBIIterator>
    uint32_t take(SamplesBIIterator samples, const Selector& selector);

 private:
    T typed_sample_;

};


#endif /* OMG_TDDS_SUB_DETAIL_DATA_READER_HPP_ */
