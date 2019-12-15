#ifndef STORE_SUBSCRIPTIONS_ERROR_H
#define STORE_SUBSCRIPTIONS_ERROR_H

#include <exception>
#include <utility>
#include <vector>

namespace reducpp {
    class store_subscriptions_error;
}

class reducpp::store_subscriptions_error : public std::exception 
{
  public:
    typedef std::pair<int /*index*/, std::exception_ptr> error;

    store_subscriptions_error(std::vector<error>&& errors) 
        : m_errors(std::move(errors)) { }

    const std::vector<error>& errors() const { return m_errors; }

  private:
    std::vector<error> m_errors;
};

#endif // STORE_SUBSCRIPTIONS_ERROR_H
