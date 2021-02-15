#include "echo.hpp"
#include "echomsg.hpp"
#include "dds/dds.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <chrono>
#include <thread>

using namespace org::eclipse::cyclonedds;

typedef ::N::S msgtype_t;

namespace cxx_test_echofunctions {

  int inttransform(int in)
  {
    return in * 2 - 1;
  }

  int inttransform_inverse(int in)
  {
    return (in + 1) / 2;
  }

  double doubletransform(double in)
  {
    return in * 0.5;
  }

  double doubletransform_inverse(double in)
  {
    return in * 2.0;
  }

  std::string stringtransform(std::string in)
  {
    //all capitalised characters to uncapitalised and inverse
    std::transform(in.begin(), in.end(), in.begin(),
      [](unsigned char c) -> unsigned char {
        if (!std::isalpha(c)) return c;
        else if (std::isupper(c)) return std::tolower(c);
        else return std::toupper(c);
      });

    //all letter characters rot13
    std::transform(in.begin(), in.end(), in.begin(),
      [](unsigned char c) -> unsigned char {
        if ((c >= 'a' && c < 'n') || (c >= 'A' && c < 'N')) return c + 13;
        else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) return c - 13;
        else return c;
      });

    return in;
  }

  int sender(const std::string& sendtopic, const std::string& recvtopic, size_t n)
  {
    try
    {
      std::cout << "=== [Sender] Create participant." << std::endl;

      dds::domain::DomainParticipant participant(domain::default_id());

      std::cout << "=== [Sender] Create topics." << std::endl;

      dds::topic::Topic<msgtype_t>
        topic1(participant, sendtopic),
        topic2(participant, recvtopic);

      std::cout << "=== [Sender] Create writer/publisher." << std::endl;

      dds::pub::Publisher publisher(participant);
      dds::pub::DataWriter<msgtype_t> writer(publisher, topic1);

      std::cout << "=== [Sender] Create reader/subscriber." << std::endl;

      dds::sub::Subscriber subscriber(participant);
      dds::sub::DataReader<msgtype_t> reader(subscriber, topic2);

      std::cout << "=== [Sender] waiting for publication match" << std::endl;

      while (writer.publication_matched_status().current_count() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

      std::cout << "=== [Sender] publication match found" << std::endl;

      size_t i = 0;
      while (i < n)
      {
        msgtype_t msg_sent(3.1415, 42+i, "Hello World");

        writer.write(msg_sent);

        //wait for messages received
        dds::sub::LoanedSamples<msgtype_t> samples;
        msgtype_t msg_recvd;
        bool waitingformsg = true;
        while (waitingformsg)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(20));

          auto samples = reader.take();
          for (const auto & sample:samples)
          {
            if (sample.info().valid())
            {
              msg_recvd = sample.data();
              waitingformsg = false;
            }
          }
        }

        /*check validity of message received*/
        if (doubletransform_inverse(msg_recvd.dbl()) != msg_sent.dbl())
          std::cout << "\ndouble inconsistency!" << doubletransform_inverse(msg_recvd.dbl()) << " != " << msg_sent.dbl() << std::endl;
        if (inttransform_inverse(msg_recvd.l()) != msg_sent.l())
          std::cout << "\nint inconsistency!" << inttransform_inverse(msg_recvd.l()) << " != " << msg_sent.l() << std::endl;
        if (stringtransform(msg_recvd.str()) != msg_sent.str())
          std::cout << "\nstring inconsistency!" << stringtransform(msg_recvd.str()) << " != " << msg_sent.str() << std::endl;

        i++;
        std::cout << "." << std::flush;
      }
      std::cout << std::endl;
      std::cout << "=== [Sender] Finished after " << i << " messages sent." << std::endl;
    }
    catch (const dds::core::Exception& e)
    {
      std::cerr << "=== [Sender] Exception: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  int answerer(const std::string& sendtopic, const std::string& recvtopic)
  {
    try
    {
      std::cout << "=== [Answerer] Create reader." << std::endl;

      dds::domain::DomainParticipant participant(domain::default_id());

      dds::topic::Topic<msgtype_t>
        topic1(participant, sendtopic),
        topic2(participant, recvtopic);

      dds::pub::Publisher publisher(participant);
      dds::pub::DataWriter<msgtype_t> writer(publisher, topic1);

      dds::sub::Subscriber subscriber(participant);
      dds::sub::DataReader<msgtype_t> reader(subscriber, topic2);

      while (writer.publication_matched_status().current_count() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

      size_t i = 0;
      while (writer.publication_matched_status().current_count()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        auto samples = reader.take();
        for (const auto& sample : samples)
        {
          msgtype_t msg_recvd = sample.data();

          if (sample.info().valid())
          {
            msg_recvd.dbl() = doubletransform(msg_recvd.dbl());
            msg_recvd.l() = inttransform(msg_recvd.l());
            msg_recvd.str(stringtransform(msg_recvd.str()));

            writer.write(msg_recvd);

            i++;
          }
        }
      }

      std::cout << "=== [Answerer] Finished after " << i << " messages received." << std::endl;
    }
    catch (const dds::core::Exception& e)
    {
      std::cerr << "=== [Answerer] Exception: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
}
