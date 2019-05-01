#ifndef ACTIVE_OBJECT_HPP
#define ACTIVE_OBJECT_HPP

#include "exception_handling_error.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <type_traits>
#include <functional>

namespace reducpp
{
    template <class R = void>
    class active_object;
}

/**
 * @brief A more or less canonical implementation of the Active Object pattern.
 */
template <class R>
class reducpp::active_object
{
  public:
    typedef std::function<R()> job_op;

    struct job
    {
        std::promise<R> promise;
        job_op operation;
        template <class F>
        job(const F& operation) : operation(operation) { }
        template <class F>
        job(F&& operation) noexcept : operation(std::move(operation)) { }
    };

    active_object()
        : m_worker(std::bind(&active_object<R>::run, this)), m_quit(false)
    { }

    active_object(active_object&& temp) noexcept
        : m_queue(std::move(temp.m_queue))
        , m_worker(std::move(temp.m_worker))
        , m_quit(false)
    { }

    ~active_object();

    active_object(const active_object&) = delete;
    active_object& operator =(const active_object&) = delete;

    template <class F>
    std::future<R> post(const F& operation);

    template <class F>
    std::future<R> post(F&& operation);

    void shutdown();

  private:
    std::queue<job> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_available;
    std::thread m_worker;
    bool m_quit;

    void run();
};

template <class T>
static void execute(typename reducpp::active_object<T>::job& j)
{
    try
    {
        j.promise.set_value(j.operation());
    }
    catch (...)
    {
        j.promise.set_exception(std::current_exception()); // this may throw
    }
}

template <>
void execute<void>(typename reducpp::active_object<void>::job& j)
{
    try
    {
        j.operation();
        j.promise.set_value();
    }
    catch (...)
    {
        j.promise.set_exception(std::current_exception()); // this may throw
    }
}

template <class R>
reducpp::active_object<R>::~active_object()
{
    shutdown();
    m_worker.join();
}

template <class R>
void reducpp::active_object<R>::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_quit = true;
    }
    m_available.notify_one();
}

template <class R>
template <class F>
std::future<R> reducpp::active_object<R>::post(const F& operation)
{
    std::future<R> retv;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job(operation));
        retv = m_queue.back().promise.get_future();
    }
    m_available.notify_one();
    return retv;
}

template <class R>
template <class F>
std::future<R> reducpp::active_object<R>::post(F&& operation)
{
    std::future<R> retv;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job(std::move(operation)));
        retv = m_queue.back().promise.get_future();
    }
    m_available.notify_one();
    return retv;
}

template <class R>
void reducpp::active_object<R>::run()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty() || m_quit)
        {
            if (m_quit)
            {
                return;
            }
            m_available.wait(lock);
        }
        job j = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();
        try
        {
            execute<R>(j);
        }
        catch (...)
        {
            std::throw_with_nested(exception_handling_error(
                "unable to properly handle exception in thread, aborted"));
        }
    }
}

#endif // ACTIVE_OBJECT_HPP
