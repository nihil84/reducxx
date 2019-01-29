#ifndef REDUCPP_STORE_H
#define REDUCPP_STORE_H

#include "composer.hpp"
#include <functional>
#include <list>
#include <vector>
#include <tuple>


namespace reducpp { 
    template <class S, class A>
    class store;

    template <class A>
    struct store_factory;
}

template <class S, class A>
class reducpp::store {
public:
    typedef std::function<S(const S&, const A&)> reducer_t;
    typedef std::function<void()> callback_t;
    
    template <class F>
    store(const F& reducer) : m_reducer(reducer) {
        m_history.push_back(S());
    }
    
    const S& state() const { return m_history.back(); }
    
    template <size_t I>
    const std::tuple_element_t<I, S>& state() { return std::get<I>(state()); }

    template <class T>
    const T& state() { return std::get<T>(state()); }

    void dispatch(const A& action);
    
    bool revert();

    template <class F>
    void subscribe(const F& callback) { m_subscriptions.push_back(callback); }
  
private:
    const reducer_t m_reducer;
    std::list<S> m_history;
    std::vector<callback_t> m_subscriptions;
};


template <class S, class A>
void reducpp::store<S, A>::dispatch(const A& action) {
    m_history.push_back(m_reducer(m_history.back(), action));
    for (const callback_t& callback : m_subscriptions) {
        callback();
    }
}

template <class S, class A>
bool reducpp::store<S, A>::revert() {
    if (m_history.size() == 1) {
        return false;
    } else {
        m_history.pop_back();
        return true;
    }
}

template <class A>
struct reducpp::store_factory {
    template <class ...Reducers>
    static auto make_store(const Reducers& ...reducers) {
        using CompositeState = typename composer<A, Reducers...>::CompositeState;
        return store<CompositeState, A>(reduce<A>::with(reducers...));
    }
};

#endif // REDUCPP_STORE_H
