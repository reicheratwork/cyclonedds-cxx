#pragma once

#include <memory>
#include "dds/dds.hpp"

/*  be very careful with this approach, as the lifetime of the entities inside this class are only as long as the
    supplied DomainParticipant, and this class does not extend its lifetime */

namespace example {

template<typename T>
class container {
  /* checking whether we can even create a Topic of this*/
  static_assert(dds::topic::is_topic_type<T>::value != 0, "not a DDS datatype");

  public:
    /* call this function to start the lifetime of the singleton */
    static void create(dds::domain::DomainParticipant &part, const std::string &topic_name) {
      if (_cont)
        dds::core::PreconditionNotMetError("Cannot initialize container class which is already initialized,");
      else
        _cont = std::unique_ptr<container<T>>(new container<T>(part, topic_name));
    }

    /* call this function to end the lifetime an instance of the singleton */
    static void destruct() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot destruct container class which is not initialized.");
      else
        _cont.reset();
    }

    /* gets the topic */
    static dds::topic::Topic<T>& get_topic() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot get topic from container class which is not initialized.");
      else
        return _cont->_topic;
    }

    /* gets the publisher */
    static dds::pub::Publisher& get_publisher() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot get publisher from container class which is not initialized.");
      else
        return _cont->_pub;
    }

    /* gets the writer, creates one if it does not exist already */
    static dds::pub::DataWriter<T>& get_writer() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot get writer from container class which is not initialized.");
      else if (_cont->_writer == dds::core::null)
         _cont->_writer = dds::pub::DataWriter<T>(get_publisher(), get_topic());
      return _cont->_writer;
    }

    /* gets the subscriber */
    static dds::sub::Subscriber& get_subscriber() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot get subscriber from container class which is not initialized.");
      else
        return _cont->_sub;
    }

    /* gets the reader, creates one if it does not exist already */
    static dds::sub::DataReader<T>& get_reader() {
      if (!_cont)
        throw dds::core::NullReferenceError("Cannot get reader from container class which is not initialized.");
      else if (_cont->_reader == dds::core::null)
         _cont->_reader = dds::sub::DataReader<T>(get_subscriber(), get_topic());
      return _cont->_reader;
    }

  private:

    /* constructor is private, to make sure everything so instances of this can only be created through calls to "create" */
    container(dds::domain::DomainParticipant &part, const std::string topic_name): _topic(part, topic_name), _pub(part), _sub(part) {
    }

    /* singleton pointer, also makes sure that everything is cleaned up correctly */
    static std::unique_ptr<container<T>> _cont;

    dds::topic::Topic<T> _topic;  /* topic gets created always */
    dds::pub::Publisher _pub; /* publisher gets created always */
    dds::sub::Subscriber _sub; /* subscriber gets created always */
    dds::pub::DataWriter<T> _writer = dds::core::null; /* lazy initialization of writer */
    dds::sub::DataReader<T> _reader = dds::core::null; /* lazy initialization of reader */
};

/* initialize to null, to indicate no singleton exists*/
template<typename T>
std::unique_ptr<container<T>> container<T>::_cont = nullptr;

}  //example
