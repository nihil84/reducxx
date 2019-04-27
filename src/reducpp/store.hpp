#ifndef REDUCPP_STORE_H
#define REDUCPP_STORE_H

#include "composer.hpp"
#include "store_subscriptions_error.hpp"
#include <functional>
#include <list>
#include <vector>
#include <tuple>

namespace reducpp
{
    template <class S, class A>
    class store;
}

/**
 * @brief Plain/basic ReduCpp store with no concurrency support.
 */
template <class S, class A>
class reducpp::store
{
  public:
    typedef std::function<S(const S &, const A &)> reducer_t;
    typedef std::function<void()> callback_t;

    template <class F>
    store(const F& reducer) : m_reducer(reducer)
    {
        m_history.push_back(S());
    }

    //! Move constructor (used for store_factory facilities)
    store(store&& temp) noexcept; 

    store(const store&) = delete;
    store& operator=(const store&) = delete;

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
    void perform_callbacks();

  private:
    const reducer_t m_reducer;
    std::list<S> m_history;
    std::vector<callback_t> m_subscriptions;
};

template <class S, class A>
reducpp::store<S, A>::store(store&& temp) noexcept
    : m_reducer(std::move(temp.m_reducer))
    , m_history(std::move(temp.m_history))
    , m_subscriptions(std::move(temp.m_subscriptions))
{ }

template <class S, class A>
void reducpp::store<S, A>::dispatch(const A& action)
{
    m_history.push_back(m_reducer(m_history.back(), action));
    perform_callbacks();
}

template <class S, class A>
bool reducpp::store<S, A>::revert()
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
void reducpp::store<S, A>::perform_callbacks()
{
    std::vector<store_subscriptions_error::error> exceptions;
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
        throw store_subscriptions_error(std::move(exceptions));
    }
}

#endif // REDUCPP_STORE_H
