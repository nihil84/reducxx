#include <reducpp/store.hpp>
#include <reducpp/action.hpp>
#include "../doctest.h"
#include <vector>

using namespace std;
using namespace reducpp;



TEST_CASE("subscriptions") {

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

    store<mystate, myaction> sut([](const mystate& state, const myaction& action) {
        mystate newstate = state;
        switch (action.type()) {
            case myaction::INCREMENT: newstate.value++; break;
            case myaction::DECREMENT: newstate.value--; break;
        }
        return std::move(newstate);
    });

    SUBCASE("GIVEN a store and a subscriber WHEN an event is dispatched THEN the subscriber is being called") {
        bool called = false;
        sut.subscribe([&]() { called = true; });

        CHECK(!called);
        sut.dispatch( { myaction::INCREMENT } );
        CHECK(called);
    }

 
}