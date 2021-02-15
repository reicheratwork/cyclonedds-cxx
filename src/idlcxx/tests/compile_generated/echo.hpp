#ifndef _echo_hpp_
#define _echo_hpp_

#include <string>

namespace cxx_test_echofunctions {
  int inttransform(int in);
  int inttransform_inverse(int in);
  double doubletransform(double in);
  double doubletransform_inverse(double in);
  std::string stringtransform(std::string in);

  int sender(const std::string& sendtopic, const std::string& recvtopic, size_t n);
  int answerer(const std::string& sendtopic, const std::string& recvtopic);
}

#endif
