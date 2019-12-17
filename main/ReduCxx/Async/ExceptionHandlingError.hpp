#ifndef REDUCXX_EXCEPTION_HANDLING_ERROR_HPP
#define REDUCXX_EXCEPTION_HANDLING_ERROR_HPP

#include <stdexcept>

namespace ReduCxx
{
    class ExceptionHandlingError;
}

class ReduCxx::ExceptionHandlingError : public std::runtime_error
{
  public:
    explicit ExceptionHandlingError(const char* msg) : runtime_error(msg) {}
};

#endif //REDUCXX_EXCEPTION_HANDLING_ERROR_HPP