#ifndef REDUCPP_STORE_H
#define REDUCPP_STORE_H

#include "action.hpp"
#include <functional>
#include <stack>


namespace reducpp { 
    template <typename S, typename A>
    class store;
}

template <typename S, typename A>
class reducpp::store {
public:
    typedef std::function<S(const S&, const A&)> reducer_t;
    
    template <typename F>
    store(const F& reducer) : m_reducer(reducer) {
        m_state.push(S());
    }
    
    const S& state() const { return m_state.top(); }
    
    void dispatch(const A& action);
    
    S revert();
  
private:
    const reducer_t m_reducer;
    std::stack<S> m_state;
};


template <typename S, typename A>
void reducpp::store<S, A>::dispatch(const A& action) {
    m_state.push(m_reducer(m_state.top(), action));
}

template <typename S, typename A>
S reducpp::store<S, A>::revert() {
    const S& top = m_state.top();
    if (m_state.size() > 1) {
        m_state.pop();
    }
    return std::move(top);
}



#endif // REDUCPP_STORE_H