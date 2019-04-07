#ifndef ACTIVE_OBJECT_HPP
#define ACTIVE_OBJECT_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

namespace reducpp {
    template <class E>
    class active_object;
}

/**
 * @brief A more or less canonical implementation of the ActiveObject pattern.
 */
template <class R>
class reducpp::active_object {
public:

    typedef std::function<void(const R&)> consumer;

    template <typename F>
    active_object(const F& consumer) 
        : m_consumer(consumer)
        , m_worker(std::bind(&active_object<R>::run, this)) 
        , m_quit(false)
    { }

    ~active_object();

    void schedule(const R& request);

    void shutdown();

private:
    std::queue<R> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_available;
    std::thread m_worker;
    consumer m_consumer;
    bool m_quit;

    void run();
};

template <class R> 
reducpp::active_object<R>::~active_object() {
    shutdown();
    m_worker.join();
}

template <class R> 
void reducpp::active_object<R>::shutdown() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_quit = true;
    }
    m_available.notify_one();
}

template <class R> 
void reducpp::active_object<R>::schedule(const R& request) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(request);
    }
    m_available.notify_one();
}

template <class R>
void reducpp::active_object<R>::run() {
    for (;;) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            if (m_quit) {
                return;
            }
            m_available.wait(lock);
        }
        R request = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();
        m_consumer(request);
    }
}


#endif // ACTIVE_OBJECT_HPP
