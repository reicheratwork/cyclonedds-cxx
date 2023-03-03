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
#ifndef CYCLONEDDS_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_
#define CYCLONEDDS_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_

/**
 * @file
 */

#include <dds/sub/LoanedSamples.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace detail
{

template <typename T>
class SharedSamples
{
public:
    /**
     * @brief Typedef for brevity purposes.
     */
    typedef typename std::vector< dds::sub::Sample<T, Sample> >::iterator iterator;
    /**
     * @brief Typedef for brevity purposes.
     */
    typedef typename std::vector< dds::sub::Sample<T, Sample> >::const_iterator const_iterator;

public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty SharedSamples container.
     */
    SharedSamples() { }

    SharedSamples(dds::sub::LoanedSamples<T> ls) : samples_(ls) { }

    /**
     * @brief Destructs an instance of SharedSamples.
     *
     * Destructs all samples stored in this class's container.
     */
    ~SharedSamples()
    {

    }

public:

    /**
     * @brief Non-const version of the iterator pointing to the beginning of the container.
     *
     * @return iterator The iterator to the beginning of the container.
     */
    iterator mbegin()
    {
        return samples_->begin();
    }

    /**
     * @brief Const version of the iterator pointing to the beginning of the container.
     *
     * @return const_iterator The iterator to the beginning of the container.
     */
    const_iterator begin() const
    {
        return samples_.begin();
    }

    /**
     * @brief Const version of the iterator pointing to the end of the container.
     *
     * @return const_iterator The iterator to the beginning of the container.
     */
    const_iterator end() const
    {
        return samples_.end();
    }

    /**
     * @brief Gets the number of samples in the container.
     *
     * @return length The number of samples in the container.
     */
    uint32_t length() const
    {
        /** @internal @todo Possible RTF size issue ? */
        return static_cast<uint32_t>(samples_.length());
    }

    /**
     * @brief Sets the number of samples in the container.
     *
     * @param[in] s The new size of the container.
     */
    void resize(uint32_t s)
    {
        samples_.resize(s);
    }

private:
    dds::sub::LoanedSamples<T> samples_;
};

}
}
}


// End of implementation

#endif /* CYCLONEDDS_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_ */
