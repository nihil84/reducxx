#include "reducpp/store.hpp"

#include <iostream>

using namespace std;

struct mystate {
  int value;
};

class myaction : public reducpp::action {
public:
  enum TYPE { INCREMENT, DECREMENT };
  myaction(TYPE type) : m_type(type) { }
  int type() const { return m_type; }
private:
  TYPE m_type;
};

mystate nop_reducer(const mystate& state, const myaction& action) {
  mystate newstate = state;
  switch (action.type()) {
    case myaction::INCREMENT: newstate.value++; break;
    case myaction::DECREMENT: newstate.value--; break;
  }
  return std::move(newstate);
}


int main(int argc, char* argv[]) {

  using namespace reducpp;
  
  store<mystate, myaction> mystore(nop_reducer);
  
  mystore.dispatch(myaction(myaction::INCREMENT));
  cout << "state: " << mystore.state().value << endl;
  mystore.dispatch(myaction(myaction::INCREMENT));
  cout << "state: " << mystore.state().value << endl;
  mystore.dispatch(myaction(myaction::INCREMENT));
  cout << "state: " << mystore.state().value << endl;
  mystore.dispatch(myaction(myaction::DECREMENT));
  cout << "state: " << mystore.state().value << endl;
  
  while (mystore.state().value > 1) {
    mystore.revert();
    cout << "state: " << mystore.state().value << endl;  
  }

  return EXIT_SUCCESS;
}
