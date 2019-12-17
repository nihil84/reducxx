#ifndef EXCEPTION_HANDLING_ERROR_HPP
#define EXCEPTION_HANDLING_ERROR_HPP

#include <stdexcept>

namespace ReduCxx
{
    class exception_handling_error;
}

class ReduCxx::exception_handling_error : public std::runtime_error
{
  public:
    exception_handling_error(const char* msg) : runtime_error(msg) {}
};

#endif // EXCEPTION_HANDLING_ERROR_HPP