#ifndef REDUCPP_STORE_H
#define REDUCPP_STORE_H

#include "action.hpp"
#include <functional>
#include <stack>


namespace reducpp { 
  template <typename S>
  class store;
}

template <typename S>
class reducpp::store {
public:
  typedef std::function<S(const S&, const action&)> reducer_t;
  
  template <typename F>
  store(const F& reducer) : m_reducer(reducer) {}
  
  const S& state() const { return m_state.top(); }
  
  void dispatch(const action& action);
  
private:
  const reducer_t m_reducer;
  std::stack<S> m_state;
};


template <typename S>
void reducpp::store<S>::dispatch(const action& action) {
  m_state.push(reducer(m_state.top(), action));
}


#endif // REDUCPP_STORE_H