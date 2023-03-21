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


/**
 * @file
 */

#ifndef CYCLONEDDS_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_
#define CYCLONEDDS_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_

#include <dds/domain/DomainParticipant.hpp>
#include <org/eclipse/cyclonedds/core/ObjectDelegate.hpp>

namespace org
{
namespace eclipse
{
namespace cyclonedds
{
namespace topic
{

/**
  * @brief Topic helper class.
  *
  * The TopicDescriptionDelegate helps the Topic and derived classes (ContentFilteredTopic, MultiTopic) by
  * storing common data.
  */
class OMG_DDS_API TopicDescriptionDelegate : public virtual org::eclipse::cyclonedds::core::DDScObjectDelegate
{
public:
    /**
      * @brief Convenience typedef,
      */
    typedef ::dds::core::smart_ptr_traits< TopicDescriptionDelegate >::ref_type ref_type;
    /**
      * @brief Convenience typedef,
      */
    typedef ::dds::core::smart_ptr_traits< TopicDescriptionDelegate >::weak_ref_type weak_ref_type;

public:
    /**
      * @brief Constructor.
      *
      * @param[in] dp domainparticipant this topic is on.
      * @param[in] name name of the topic.
      * @param[in] type_name name of the datatype of the topic.
      */
    TopicDescriptionDelegate(const dds::domain::DomainParticipant& dp,
                             const std::string& name,
                             const std::string& type_name);
    /**
      * @brief Destructor (virtual).
      */
    virtual ~TopicDescriptionDelegate();

public:

    /**
      *  @brief Topic name getter.
      *
      * @return const std::string& The name of the topic.
      */
    const std::string& name() const;

    /**
      *  @brief Topic datatype name getter.
      *
      * @return const std::string& The name of the datatype.
      */
    const std::string& type_name() const;

    /**
      *  @brief DomainParticipant getter.
      *
      * @return const dds::domain::DomainParticipant& The domainparticipant the topic is on.
      */
    const dds::domain::DomainParticipant& domain_participant() const;

    /**
      * @brief Dependents incrementer.
      *
      * Increments the number of dependents.
      * Dependents are subordinate entities of Topics, so DataWriters, DataReaders and ContentFilteredTopics.
      */
    void incrNrDependents();

    /**
      * @brief Dependents decrementer.
      *
      * Decrements the number of dependents.
      * Dependents are subordinate entities of Topics, so DataWriters, DataReaders and ContentFilteredTopics.
      */
    void decrNrDependents();

    /**
      * @brief Dependents check function.
      *
      * Checks whether this Topic has dependents.
      * Dependents are subordinate entities of Topics, so DataWriters, DataReaders and ContentFilteredTopics.
      *
      * @retval true When there are more than 0 dependents.
      * @retval false When there are 0 dependents.
      */
    bool hasDependents() const;

    virtual std::string reader_expression() const = 0;

    //@todo virtual c_value *reader_parameters() const = 0;

    /**
      * @brief C sertype getter.
      *
      * Returns the CycloneDDS-C sertype information for this topic.
      *
      * @return ddsi_sertype* pointer to the sertype information for this topic.
      */
    ddsi_sertype *get_ser_type() const;

protected:
    dds::domain::DomainParticipant myParticipant;
    std::string myTopicName;
    std::string myTypeName;
    uint32_t nrDependents;
    ddsi_sertype *ser_type_;
};

}
}
}
}

#endif /* CYCLONEDDS_TOPIC_TOPICDESCRIPTIONDELEGATE_HPP_ */
