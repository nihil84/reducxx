#include <reducpp/store.hpp>
#include <reducpp/action.hpp>
#include <reducpp/composer.hpp>
#include "../doctest.h"
#include <vector>

using namespace std;
using namespace reducpp;

namespace composite_state_test {

class myaction : public reducpp::action {
public:
    enum TYPE { INCREMENT, DECREMENT };
    myaction(TYPE type) : m_type(type) { }
    int type() const { return m_type; }
private:
    TYPE m_type;
};    

struct mystate1 {
    int value;
};

struct mystate2 {
    int value;
    mystate2 reducer(const mystate2& state, const myaction& action) {
        return { state.value +1 };
    }
};

struct mystate3 {
    vector<int> ints;
};

mystate1 dummy_reducer(const mystate1& state, const myaction& action) {
    mystate1 newstate = state;
    switch (action.type()) {
        case myaction::INCREMENT: newstate.value++; break;
        case myaction::DECREMENT: newstate.value--; break;
    }
    return std::move(newstate);
}

mystate3 nop_reducer(const mystate3& state, const myaction& action) {
    return { {3, 4, 5} };
}

} // end of namespace

TEST_CASE("composite state") {

    using namespace composite_state_test;
    using namespace std::placeholders;

    mystate2 instance;
    auto member_fun = std::bind(&mystate2::reducer, &instance, _1, _2);

    function<mystate1(const mystate1&, const myaction&)> dummy(dummy_reducer);

    auto reducers = reduce<myaction>::with();
 
 //   store<std::tuple<mystate1, mystate2, mystate3>, myaction> sut(reducers);
    auto sut = store_factory<myaction>::make_store(dummy, member_fun, nop_reducer);

    CHECK(sut.state<mystate1>().value == 0);
    CHECK(sut.state<mystate2>().value == 0);
    CHECK(sut.state<mystate3>().ints.size() == 0);
    sut.dispatch(myaction(myaction::DECREMENT));
    CHECK(sut.state<0>().value == -1);
    CHECK(sut.state<1>().value == 1);
    CHECK(sut.state<2>().ints.size() == 3);    
}