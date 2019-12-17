#include <ReduCxx/Store.hpp>
#include <ReduCxx/Action.hpp>
#include "../catch.hpp"
#include <vector>

using namespace std;
using namespace ReduCxx;


SCENARIO("subscriptions") {

    struct mystate {
        int value;
    };

    class myaction : public ReduCxx::Action {
    public:
        enum TYPE { INCREMENT, DECREMENT };
        myaction(TYPE type) : m_type(type) { }
        int type() const { return m_type; }
    private:
        TYPE m_type;
    };

    Store<mystate, myaction> sut([](const mystate& state, const myaction& action) {
        mystate newstate = state;
        switch (action.type()) {
            case myaction::INCREMENT: newstate.value++; break;
            case myaction::DECREMENT: newstate.value--; break;
        }
        return newstate;
    });

    GIVEN("a Store and a subscriber")
    WHEN("an event is dispatched") 
    THEN("the subscriber is being called") {
        bool called = false;
        sut.subscribe([&]() { called = true; });

        CHECK(!called);
        sut.dispatch( { myaction::INCREMENT } );
        CHECK(called);
    }

 
}
