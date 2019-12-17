#ifndef REDUCXX_ACTIVE_OBJECT_HPP
#define REDUCXX_ACTIVE_OBJECT_HPP

#include "ExceptionHandlingError.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <type_traits>
#include <functional>

namespace ReduCxx
{
    template <class R = void>
    class ActiveObject;
}

/**
 * @brief A more or less canonical implementation of the Active Object pattern.
 */
template <class R>
class ReduCxx::ActiveObject
{
  public:
    typedef std::function<R()> job_op;

    struct job
    {
        std::promise<R> promise;
        job_op operation;
        template <class F>
        explicit job(const F& operation) : operation(operation) { }
        template <class F>
        job(F&& operation) noexcept : operation(std::forward<F>(operation)) { }
        job(job&& rhs) noexcept : promise(std::move(rhs.promise)), operation(std::move(rhs.operation)) { }
        job(const job&) = delete;
        job& operator =(const job&) = delete;
    };

    ActiveObject()
        : m_worker(std::bind(&ActiveObject<R>::run, this)), m_quit(false)
    { }

    ActiveObject(ActiveObject&& temp) noexcept
        : m_queue(std::move(temp.m_queue))
        , m_worker(std::move(temp.m_worker))
        , m_quit(false)
    { }

    ~ActiveObject();

    ActiveObject(const ActiveObject&) = delete;
    ActiveObject& operator =(const ActiveObject&) = delete;

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
static void execute(typename ReduCxx::ActiveObject<T>::job& j)
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

#include <iostream>
template <>
void execute<void>(typename ReduCxx::ActiveObject<void>::job& j)
{
    static int count = 0;
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
ReduCxx::ActiveObject<R>::~ActiveObject()
{
    shutdown();
    m_worker.join();
}

template <class R>
void ReduCxx::ActiveObject<R>::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_quit = true;
    }
    m_available.notify_one();
}

template <class R>
template <class F>
std::future<R> ReduCxx::ActiveObject<R>::post(const F& operation)
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
std::future<R> ReduCxx::ActiveObject<R>::post(F&& operation)
{
    std::future<R> retv;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(job(std::forward<F>(operation)));
        retv = m_queue.back().promise.get_future();
    }
    m_available.notify_one();
    return retv;
}

template <class R>
void ReduCxx::ActiveObject<R>::run()
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
            // this will terminate the application
            std::throw_with_nested(ExceptionHandlingError(
                "unable to properly handle exception in thread, aborted"));
        }
    }
}

#endif //REDUCXX_ACTIVE_OBJECT_HPP
