#ifndef EXCEPTION_HANDLING_ERROR_HPP
#define EXCEPTION_HANDLING_ERROR_HPP

#include <stdexcept>

namespace reducpp
{
    template <class T>
    class exception_handling_error;
}

template <class T>
class reducpp::exception_handling_error : public std::runtime_error
{
  public:
    exception_handling_error(const char* msg, const T &data) : runtime_error(msg), m_data(data) {}
    exception_handling_error(const char* msg, T &&data) : runtime_error(msg), m_data(std::move(data)) {}

    const T &data() { return m_data; }

  private:
    T m_data;
};

#endif // EXCEPTION_HANDLING_ERROR_HPP