#ifndef REDUCXX_STORE_SUBSCRIPTIONS_ERROR_HPP
#define REDUCXX_STORE_SUBSCRIPTIONS_ERROR_HPP

#include <exception>
#include <utility>
#include <vector>

namespace ReduCxx {
    class StoreSubscriptionsError;
}

class ReduCxx::StoreSubscriptionsError : public std::exception
{
  public:
    typedef std::pair<int /*index*/, std::exception_ptr> error;

    explicit StoreSubscriptionsError(std::vector<error>&& errors)
        : m_errors(std::move(errors)) { }

    [[nodiscard]] const std::vector<error>& errors() const { return m_errors; }

  private:
    std::vector<error> m_errors;
};

#endif //REDUCXX_STORE_SUBSCRIPTIONS_ERROR_HPP
