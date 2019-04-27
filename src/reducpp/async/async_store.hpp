#ifndef ASYNC_STORE_HPP
#define ASYNC_STORE_HPP

#include "../store.hpp"
#include "active_object.hpp"

#include <thread>

namespace reducpp {
    template <class S, class A>
    class async_store;
}

template <class S, class A>
class reducpp::async_store {
public:

    template <class F>
    async_store(const F& reducer) 
        : m_store(reducer)
        , m_worker(std::bind(&async_store::do_dispatch, this, std::placeholders::_1)) 
    { }

    async_store(async_store&& temp) noexcept
        : m_store(std::move(temp.m_store))
        , m_worker(std::move(temp.m_worker))
    { }

    async_store(const async_store&) = delete;
    async_store& operator =(const async_store&) = delete;

    /**
     * @brief Process action dispatching on a separate thread, returning a @a future to check for completion.
     * 
     * The returned future can be ignored but, in case of exception, it will pass undetected:
     * If a reducer throws, the exception is bounded to the future an rethrown on future.get(), state is left unchanged.
     */
    std::future<void> dispatch(const A& action) { return m_worker.post(action); }
    std::future<void> dispatch(A&& action) { return m_worker.post(std::move(action)); }

private:
    store<S, A> m_store;
    active_object<A, void> m_worker;

    void do_dispatch(const A& action);
};


template <class S, class A>
void reducpp::async_store<S, A>::do_dispatch(const A& action)
{
    m_store.dispatch(action);
}

#endif // ASYNC_STORE_HPP