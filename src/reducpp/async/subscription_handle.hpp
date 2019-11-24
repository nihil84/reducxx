#ifndef REDUCPP_SUBSCRIPTION_HANDLE_HPP
#define REDUCPP_SUBSCRIPTION_HANDLE_HPP

#include <future>
#include <queue>

namespace reducpp {
    class subscription_handle;
}

class reducpp::subscription_handle {
public:

    subscription_handle(const subscription_handle&) = delete;
    subscription_handle& operator =(const subscription_handle&) = delete;

    subscription_handle() = default;

    inline int count() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_promises.size();
    }

    inline void add(std::future<void>&& result) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_promises.push_back(std::move(result));
    }

    inline void wait_one() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_waiter.wait(lock, [&]() { return m_promises.size() > 0; });
        std::future<void> one(std::move(*m_promises.begin()));
        m_promises.pop_front();
        one.get();
    }

    inline void wait_all() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_waiter.wait(lock, [&]() { return m_promises.size() > 0; });
        while (count() > 0) {
            std::future<void> one(std::move(*m_promises.begin()));
            m_promises.pop_front();
            one.get();
        }
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_waiter;
    std::list<std::future<void>> m_promises;
};



#endif //REDUCPP_SUBSCRIPTION_HANDLE_HPP
