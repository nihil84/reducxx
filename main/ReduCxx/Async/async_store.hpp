#ifndef ASYNC_STORE_HPP
#define ASYNC_STORE_HPP

#include "ReduCxx/Store.hpp"
#include "active_object.hpp"
#include "subscription_handle.hpp"

#include <thread>

namespace ReduCxx {
    template <class S, class A>
    class async_store;
}

/**
 * @brief Asynchronous Store.
 * This Store runs reducers on its own thread and can be used when you need to
 * dispatch state changes from different threads.
 * 
 * Please be aware that reducers shall not access to shared resources.
 */
template <class S, class A>
class ReduCxx::async_store {
public:

    template <class F>
    explicit async_store(const F& reducer)
        : m_store(reducer)
    { }

    async_store(async_store&& temp) noexcept
        : m_store(std::move(temp.m_store))
        , m_reducer_thread(std::move(temp.m_reducer_thread))
    { }

    async_store(const async_store&) = delete;
    async_store& operator =(const async_store&) = delete;

    /**
     * @brief Process action dispatching on a separate thread, returning a 
     * @a future to check for completion.
     * 
     * The returned future can be ignored but, in case of exception, it will 
     * pass undetected: If a reducer throws, the exception is bounded to the 
     * future an rethrown on future.get(), state is left unchanged.
     */
    std::future<void> dispatch(const A& action);
    std::future<void> dispatch(A&& action);

    //! @brief Return a copy of current state
    S state() const;

    //! @brief Return a copy of to the sub-state of index @a I in case @a S is a std::tuple
    template <size_t I>
    std::tuple_element_t<I, S> state();

    //! @brief Return a copy to the sub-state of type @a T in case @a S is a std::tuple
    template <class T>
    T state();

    /**
     * @brief Add given function or function to the Store subscriptions for state change.
     * Subscriptions will run on the reducers thread and are then synchronous 
     * to the state change.
     * @note
     * Due to the fact the @a callback will run on the Store thread, you should avoid lengthy, time consuming
     * computations here, otherwise the event processing may slow down excessively.
     * Long computations could be moved to another thread with any custom signalling mechanism or you can use
     * a @a ReduCxx::active_object and the @a async_store::subscribe_async function directly.
     */ 
    template <class F>
    void subscribe_sync(const F& callback) {
        m_store.subscribe(callback); 
    }

    /**
     * @brief Add given function to the Store subscriptions for state changes.
     * Subscriptions will run on the given <i>active object</i>.
     * @return a reference to a heap allocated handle that collect results from each execution
     * of the subscriber, if you are not interested in them, discard or dispose the returned pointer
     * to avoid memory overload.
     */
    template <class F>
    std::shared_ptr<subscription_handle> subscribe_async(active_object<void>& subscriber, const F& op);

private:
    Store<S, A> m_store;
    mutable std::mutex m_mutex;
    active_object<void> m_reducer_thread;

    void do_dispatch(const A& action);
};

template <class S, class A>
std::future<void> ReduCxx::async_store<S, A>::dispatch(const A& action) {
    return m_reducer_thread.post(
        std::bind(&async_store<S, A>::do_dispatch, this, action));
}

template <class S, class A>
std::future<void> ReduCxx::async_store<S, A>::dispatch(A&& action) {
    return m_reducer_thread.post(
        std::bind(&async_store<S, A>::do_dispatch, this, std::move(action)));
}

template<class S, class A>
S ReduCxx::async_store<S, A>::state() const {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_store.state();
}

template<class S, class A>
template <size_t I>
std::tuple_element_t<I, S> ReduCxx::async_store<S, A>::state()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return std::get<I>(m_store.state());
}

template<class S, class A>
template<class T>
T ReduCxx::async_store<S, A>::state()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return std::get<T>(m_store.state());
}

template <class S, class A>
void ReduCxx::async_store<S, A>::do_dispatch(const A& action)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_store.dispatch(action);
}

template<class S, class A>
template<class F>
std::shared_ptr<ReduCxx::subscription_handle>
ReduCxx::async_store<S, A>::subscribe_async(ReduCxx::active_object<void> &subscriber, const F &op) {
    std::shared_ptr<subscription_handle> caller_handle(new subscription_handle);
    std::weak_ptr<subscription_handle> handler_handle = caller_handle;
    m_store.subscribe([&subscriber, op, handler_handle]() {
        std::future<void> result = subscriber.post(op);
        if (auto handle = handler_handle.lock()) {
            handle->add(std::move(result));
        }
    });
    return caller_handle;
}

#endif // ASYNC_STORE_HPP
