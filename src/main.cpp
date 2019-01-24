#include "reducpp/store.hpp"

#include <iostream>

using namespace std;

struct mystate {
  int value;
};

class myaction : public reducpp::action {
  int type() const { return 0; }
};

mystate nop_reducer(const mystate& state, const reducpp::action& action) {
  return state;
}


int main(int argc, char* argv[]) {

  using namespace reducpp;
  
  
  store<mystate> mystore(nop_reducer);
  
  
  cout << "state: " << mystore.state().value << endl;

  return EXIT_SUCCESS;
}
