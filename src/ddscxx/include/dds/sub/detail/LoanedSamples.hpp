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
#ifndef OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_
#define OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_

namespace dds
{
namespace sub
{
namespace detail
{

template <typename T>
class LoanedSamples
{
public:

    /**
     * @brief Typedef for a container of typed references to Samples.
     */
    typedef std::vector< dds::sub::SampleRef<T, dds::sub::detail::SampleRef> > LoanedSamplesContainer;

    /**
     * @brief Typedef an iterator for a LoanedSamplesContainer.
     */
    typedef typename std::vector< dds::sub::SampleRef<T, dds::sub::detail::SampleRef> >::iterator iterator;

    /**
     * @brief Typedef a const iterator for a LoanedSamplesContainer.
     */
    typedef typename std::vector< dds::sub::SampleRef<T, dds::sub::detail::SampleRef> >::const_iterator const_iterator;

public:
    /**
     * @brief Default constructor.
     */
    LoanedSamples() { }

    /**
     * @brief Destructor.
     */
    ~LoanedSamples()
    {

    }

public:

    /**
     * @brief Non-const iterator to the beginning of the container.
     *
     * @return iterator the iterator to the beginning of the container.
     */
    iterator mbegin()
    {
        return samples_.begin();
    }

    /**
     * @brief Const iterator to the beginning of the container.
     *
     * @return const_iterator the const iterator to the beginning of the container.
     */
    const_iterator begin() const
    {
        return samples_.begin();
    }

    /**
     * @brief Const iterator to the end of the container.
     *
     * @return const_iterator the const iterator to the end of the container.
     */
    const_iterator end() const
    {
        return samples_.end();
    }

    /**
     * @brief Container length getter function.
     *
     * @return uint32_t the current length of the container.
     */
    uint32_t length() const
    {
        return static_cast<uint32_t>(samples_.size());
    }

    /**
     * @brief Container reserve function.
     *
     * Reserves space for a specific number of samples, but does not assign values to them yet.
     *
     * @param[in] s the number of samples to reserve.
     */
    void reserve(uint32_t s)
    {
        samples_.reserve(s);
    }

    /**
     * @brief Container resize function.
     *
     * Makes the current container to be of a specific size, entries in excess to this will be destructed.
     * If the container is expanded, the new samples will not yet have values associated with them.
     *
     * @param[in] s the number of samples to resize the container to.
     */
    void resize(uint32_t s)
    {
         samples_.resize(s);
    }

    /**
     * @brief Container specific element access function.
     *
     * Returns a reference to the element at the specified offset.
     *
     * @param[in] i the offset at which to return the element.
     *
     * @return dds::sub::SampleRef<T, dds::sub::detail::SampleRef> reference to the element at the specified offset.
     */
    dds::sub::SampleRef<T, dds::sub::detail::SampleRef>& operator[] (uint32_t i)
    {
        return this->samples_[i];
    }

    /**
     * @brief Container buffer direct access function.
     *
     * Returns a pointer to the container's underlying array.
     *
     * @return dds::sub::SampleRef<T, dds::sub::detail::SampleRef> pointer to the underlying container's array.
     */
    dds::sub::SampleRef<T, dds::sub::detail::SampleRef> * get_buffer() {
        return this->samples_.data();
    }


private:
    LoanedSamplesContainer samples_;
};

template <>
class LoanedSamples<org::eclipse::cyclonedds::topic::CDRBlob>
{
public:

    typedef std::vector< dds::sub::Sample<org::eclipse::cyclonedds::topic::CDRBlob, dds::sub::detail::Sample> > LoanedSamplesContainer;
    typedef typename std::vector< dds::sub::Sample<org::eclipse::cyclonedds::topic::CDRBlob, dds::sub::detail::Sample> >::iterator iterator;
    typedef typename std::vector< dds::sub::Sample<org::eclipse::cyclonedds::topic::CDRBlob, dds::sub::detail::Sample> >::const_iterator const_iterator;

public:
    LoanedSamples() { }

    ~LoanedSamples()
    {

    }

public:

    iterator mbegin()
    {
        return samples_.begin();
    }

    const_iterator begin() const
    {
        return samples_.begin();
    }

    const_iterator end() const
    {
        return samples_.end();
    }

    uint32_t length() const
    {
        return static_cast<uint32_t>(samples_.size());
    }

    void reserve(uint32_t s)
    {
        samples_.reserve(s);
    }

    void resize(uint32_t s)
    {
         samples_.resize(s);
    }

    dds::sub::Sample<org::eclipse::cyclonedds::topic::CDRBlob, dds::sub::detail::Sample>& operator[] (uint32_t i)
    {
        return this->samples_[i];
    }

    dds::sub::Sample<org::eclipse::cyclonedds::topic::CDRBlob, dds::sub::detail::Sample> * get_buffer() {
        return this->samples_.data();
    }


private:
    LoanedSamplesContainer samples_;
};


}
}
}
#endif /* OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_ */
