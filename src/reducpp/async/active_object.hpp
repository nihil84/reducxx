#ifndef ACTIVE_OBJECT_HPP
#define ACTIVE_OBJECT_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <type_traits>
#include <functional>

namespace reducpp {
    template <class E, typename T = void>
    class active_object;
}

/**
 * @brief A more or less canonical implementation of the ActiveObject pattern.
 */
template <class R, typename T>
class reducpp::active_object {
public:

    struct job {
        std::promise<T> promise;
        const R request;
        job(R&& request) : request(std::move(request)) { }
    };

    typedef std::function<T(const R&)> consumer;

    template <typename F>
    active_object(const F& consumer) 
        : m_consumer(consumer)
        , m_worker(std::bind(&active_object<R>::run, this)) 
        , m_quit(false)
    { }

    ~active_object();

    std::future<T> post(const R& request);

    std::future<T> post(R&& request);

    void shutdown();

private:
    std::queue<job> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_available;
    std::thread m_worker;
    consumer m_consumer;
    bool m_quit;

    void run();
};

template <class R, typename T>
void execute(   const typename reducpp::active_object<R,T>::consumer& consumer, 
                typename reducpp::active_object<R,T>::job& j) noexcept 
{
    try {
        j.promise.set_value(consumer(j.request));
    } catch (...) {
        try { j.promise.set_exception(std::current_exception()); }
        catch (...) { /* unable to handle exception */ }
    }
}

template <class R>
void execute(   const typename reducpp::active_object<R,void>::consumer& consumer,
                typename reducpp::active_object<R,void>::job& j) noexcept 
{
    try {
        consumer(j.request);
        j.promise.set_value();
    } catch (...) {
        try { j.promise.set_exception(std::current_exception()); }
        catch (...) { /* unable to handle exception */ }
    }
}

template <class R, typename T> 
reducpp::active_object<R, T>::~active_object() {
    shutdown();
    m_worker.join();
}

template <class R, typename T> 
void reducpp::active_object<R, T>::shutdown() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_quit = true;
    }
    m_available.notify_one();
}

template <class R, typename T> 
std::future<T> reducpp::active_object<R, T>::post(const R& request) {
    std::future<T> retv;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job(request));
        retv = m_queue.back().promise.get_future();
    }
    m_available.notify_one();
    return std::move(retv);
}

template <class R, typename T> 
std::future<T> reducpp::active_object<R, T>::post(R&& request) {
    std::future<T> retv;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job(std::move(request)));
        retv = m_queue.back().promise.get_future();
    }
    m_available.notify_one();
    return std::move(retv);
}

template <class R, typename T>
void reducpp::active_object<R, T>::run() {
    for (;;) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            if (m_quit) {
                return;
            }
            m_available.wait(lock);
        }
        job j = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();
        execute<R>(m_consumer, j);
    }
}



#endif // ACTIVE_OBJECT_HPP
