#ifndef REDUCPP_STORE_H
#define REDUCPP_STORE_H

#include <functional>
#include <stack>


namespace reducpp { 
    template <class S, class A>
    class store;
}

template <class S, class A>
class reducpp::store {
public:
    typedef std::function<S(const S&, const A&)> reducer_t;
    
    template <class F>
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


template <class S, class A>
void reducpp::store<S, A>::dispatch(const A& action) {
    m_state.push(m_reducer(m_state.top(), action));
}

template <class S, class A>
S reducpp::store<S, A>::revert() {
    const S& top = m_state.top();
    if (m_state.size() > 1) {
        m_state.pop();
    }
    return std::move(top);
}



#endif // REDUCPP_STORE_H