#ifndef REDUCXX_STORE_HPP
#define REDUCXX_STORE_HPP

#include "Composer.hpp"
#include "StoreSubscriptionsError.hpp"
#include <functional>
#include <list>
#include <vector>
#include <tuple>

namespace ReduCxx
{
    template <class S, class A>
    class Store;
}

/**
 * @brief Plain/basic ReduCpp Store with no concurrency support.
 */
template <class S, class A>
class ReduCxx::Store
{
  public:
    typedef std::function<S(const S &, const A &)> reducer_t;
    typedef std::function<void()> callback_t;

    template <class F>
    explicit Store(const F& reducer) : m_reducer(reducer)
    {
        m_history.push_back(S());
    }

    //! Move constructor (used for StoreFactory facilities)
    Store(Store&& temp) noexcept;

    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    //! @brief Return a read-only reference to current state
    virtual const S& state() const { return m_history.back(); }

    //! @brief Return a read-only reference to the sub-state of index @a I in 
    //! case @a S is a std::tuple
    template <size_t I>
    const std::tuple_element_t<I, S>& state() { return std::get<I>(state()); }

    //! @brief Return a read-only reference to the sub-state of type @a T in 
    //! case @a S is a std::tuple
    template <class T>
    const T& state() { return std::get<T>(state()); }

    void dispatch(const A& action);

    bool revert();

    /**
     * @brief Subscribe given @a callback to be called at each state change.
     * If any of the callbacks throws, the exception is put in stasis until all
     * the subscriptions are run, then a special @a umbrella exception will be
     * thrown storing a list of all exceptions 
     */
    template <class F>
    void subscribe(const F& callback)
    { m_subscriptions.push_back(callback); }

  protected:
    void performCallbacks();

  private:
    const reducer_t m_reducer;
    std::list<S> m_history;
    std::vector<callback_t> m_subscriptions;
};

template <class S, class A>
ReduCxx::Store<S, A>::Store(Store&& temp) noexcept
    : m_reducer(std::move(temp.m_reducer))
    , m_history(std::move(temp.m_history))
    , m_subscriptions(std::move(temp.m_subscriptions))
{ }

template <class S, class A>
void ReduCxx::Store<S, A>::dispatch(const A& action)
{
    m_history.push_back(m_reducer(m_history.back(), action));
    performCallbacks();
}

template <class S, class A>
bool ReduCxx::Store<S, A>::revert()
{
    if (m_history.size() == 1)
    {
        return false;
    }
    else
    {
        m_history.pop_back();
        return true;
    }
}

template <class S, class A>
void ReduCxx::Store<S, A>::performCallbacks()
{
    std::vector<StoreSubscriptionsError::error> exceptions;
    int idx = 0;
    for (const callback_t& callback : m_subscriptions)
    {
        try 
        {
            callback();
        } 
        catch (...) 
        {
            exceptions.push_back(std::make_pair(idx, std::current_exception()));
        }
        ++idx;
    }

    if (!exceptions.empty())
    {
        throw StoreSubscriptionsError(std::move(exceptions));
    }
}

#endif //REDUCXX_STORE_HPP
