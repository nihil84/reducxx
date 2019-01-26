#include "reducpp/store.hpp"
#include "reducpp/action.hpp"
#include "reducpp/composer.hpp"

#include <iostream>

using namespace std;

struct mystate {
    int value;
};

struct another_state {
    int value2;
};

class myaction : public reducpp::action {
public:
    enum TYPE { INCREMENT, DECREMENT };
    myaction(TYPE type) : m_type(type) { }
    int type() const { return m_type; }
private:
    TYPE m_type;
};

mystate dummy_reducer(const mystate& state, const myaction& action) {
    mystate newstate = state;
    switch (action.type()) {
        case myaction::INCREMENT: newstate.value++; break;
        case myaction::DECREMENT: newstate.value--; break;
    }
    return std::move(newstate);
}

another_state nop_reducer(const another_state& state, const myaction& action) {
    another_state newstate = state;
    return std::move(newstate);
}



int main(int argc, char* argv[]) {

    using namespace reducpp;
    
    store<std::tuple<mystate, another_state>, myaction> mystore(
        (compose<myaction>::of<mystate, another_state>(dummy_reducer, nop_reducer)));
    
    mystore.dispatch(myaction(myaction::INCREMENT));
    cout << "state: " << std::get<0>(mystore.state()).value << endl;
    mystore.dispatch(myaction(myaction::INCREMENT));
    cout << "state: " << std::get<0>(mystore.state()).value << endl;
    mystore.dispatch(myaction(myaction::INCREMENT));
    cout << "state: " << std::get<0>(mystore.state()).value << endl;
    mystore.dispatch(myaction(myaction::DECREMENT));
    cout << "state: " << std::get<0>(mystore.state()).value << endl;
    
    while (std::get<0>(mystore.state()).value > 1) {
        mystore.revert();
        cout << "state: " << std::get<0>(mystore.state()).value << endl;  
    }

    return EXIT_SUCCESS;
}
