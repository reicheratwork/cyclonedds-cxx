#ifndef OMG_DDS_PUB_DETAIL_SAMPLEREF_HPP_
#define OMG_DDS_PUB_DETAIL_SAMPLEREF_HPP_

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


namespace dds
{
namespace sub
{
namespace detail
{
template <typename T>
class SampleRef;
}}}

#include <dds/sub/SampleInfo.hpp>
#include <org/eclipse/cyclonedds/topic/datatopic.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
template <typename T>
class SampleRef
{
public:
    /**
     * @brief Default constructor.
     *
     * Will the data pointer to null, indicating that no data is being referenced right now.
     *
     * @return SampleRef the constructed instance.
     */
    SampleRef()
    {
      this->data_ = nullptr;
    }

    /**
     * @brief Initialized constructor.
     *
     * @param[in] d the data to be referenced by this entity.
     * @param[in] i the sample information associated with this data.
     *
     * @return SampleRef the constructed instance.
     */
    SampleRef(ddscxx_serdata<T>* d, const dds::sub::SampleInfo& i)
    {
        this->data_ = d;
        this->info_ = i;
    }

    /**
     * @brief Copy constructor.
     *
     * Will increase the references to the copied data.
     *
     * @param[in] other the instance to copy.
     *
     * @return SampleRef the constructed instance.
     */
    SampleRef(const SampleRef& other)
    {
        copy(other);
    }

    /**
     * @brief Destructor.
     *
     * Will unref the contained data, if any.
     */
    virtual ~SampleRef()
    {
      if (data_ != nullptr) {
        ddsi_serdata_unref(reinterpret_cast<ddsi_serdata *>(data_));
      }
    }

    /**
     * @brief Copy assignment operator.
     *
     * Will copy the data from the other instance, and increase the references to the data.
     *
     * @param[in] other the instance to copy.
     *
     * @return SampleRef& the assigned instance.
     */
    SampleRef& operator=(const SampleRef& other)
    {
      if (this != &other)
      {
          copy(other);
      }
      return *this;
    }

public:
    /**
     * @brief Data field accessor.
     *
     * Will check whether data is associated with this instance and throw an error if there isn't.
     *
     * @throw dds::core::Error when there is no data associated with this instance.
     *
     * @return T& reference to the datafield.
     */
    const T& data() const
    {
      if (data_ == nullptr)
      {
        throw dds::core::Error("Data is Null");
      }
      return *data_->getT();
    }

    /**
     * @brief Sample info accessor (const).
     *
     * Will return the sample info associated with this instance.
     * This accessor does not allow modifications.
     *
     * @return SampleInfo& const reference to the sample info.
     */
    const dds::sub::SampleInfo& info() const
    {
        return info_;
    }

    /**
     * @brief Sample info accessor.
     *
     * Will return the sample info associated with this instance.
     * This accessor allows modifications.
     *
     * @return SampleInfo& reference to the sample info.
     */
    dds::sub::SampleInfo& info()
    {
        return info_;
    }

    /**
     * @brief Comparison operator.
     *
     * @param[in] other other instance to compare with, is not used.
     *
     * @retval false always.
     */
    bool operator ==(const SampleRef& other) const
    {
        (void)other;
        return false;
    }

    /**
     * @brief Data field address accessor.
     *
     * Allows access and modification of the data field in this instance.
     *
     * @retval ddscxx_serdata<T>*& reference to the pointer to the data in this instance.
     */
    ddscxx_serdata<T>* &data_ptr()
    {
        return this->data_;
    }


private:
    void copy(const SampleRef& other)
    {
        if (other.data_ == nullptr)
        {
            throw dds::core::Error("Other data is Null");
        }
        static_cast<void>(ddsi_serdata_ref(reinterpret_cast<ddsi_serdata* const>(other.data_)));
        this->data_ = other.data_;
        this->info_ = other.info_;
    }

    ddscxx_serdata<T>* data_;
    dds::sub::SampleInfo info_;
};

}
}
}

#endif /* OMG_DDS_PUB_DETAIL_SAMPLEREF_HPP_ */
