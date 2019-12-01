#ifndef REDUCPP_SUBSCRIPTION_HANDLE_HPP
#define REDUCPP_SUBSCRIPTION_HANDLE_HPP

#include <future>
#include <queue>
#include <chrono>

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
        return m_futures.size();
    }

    inline void add(std::future<void>&& result) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_futures.push_back(std::move(result));
    }

    inline void wait_one() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_waiter.wait(lock, [&]() { return !m_futures.empty(); });
        pop();
    }

    template <class Rep, class Period>
    bool wait_one(const std::chrono::duration<Rep,Period>& timeout);

    inline void wait_all() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_futures.empty()) {
            pop();
        }
    }

    template <class Rep, class Period>
    bool wait_all(const std::chrono::duration<Rep,Period>& timeout);

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_waiter;
    std::list<std::future<void>> m_futures;

    inline void pop() {
        std::future<void> one(std::move(*this->m_futures.begin()));
        this->m_futures.pop_front();
        one.get();
    }
};

template<class Rep, class Period>
bool reducpp::subscription_handle::wait_one(const std::chrono::duration<Rep, Period>& timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_waiter.wait(lock, [&]() { return !m_futures.empty(); });
    if (m_futures.begin()->wait_for(timeout) != std::future_status::ready) return false;
    pop();
    return true;
}

template<class Rep, class Period>
bool reducpp::subscription_handle::wait_all(const std::chrono::duration<Rep, Period>& timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_futures.empty()) {
        if (m_futures.begin()->wait_for(timeout) != std::future_status::ready) return false;
        pop();
    }
    return true;
}


#endif //REDUCPP_SUBSCRIPTION_HANDLE_HPP
