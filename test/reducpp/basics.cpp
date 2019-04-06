#include <reducpp/store.hpp>
#include <reducpp/action.hpp>
#include "../doctest.h"

using namespace reducpp;


TEST_CASE("basic functionalities") {

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

    SUBCASE("GIVEN a store WHEN dispatching an event THEN the state is updated") {
        CHECK(sut.state().value == 0);
        sut.dispatch(myaction(myaction::INCREMENT));
        CHECK(sut.state().value == 1);
        sut.dispatch(myaction(myaction::INCREMENT));
        CHECK(sut.state().value == 2);
        sut.dispatch(myaction(myaction::DECREMENT));
        CHECK(sut.state().value == 1);
    }

    SUBCASE("GIVEN a store with an history of N actions WHEN reverting THEN the state is rolled-back") {
        CHECK(sut.state().value == 0);
        sut.dispatch(myaction(myaction::INCREMENT));
        sut.dispatch(myaction(myaction::INCREMENT));
        sut.dispatch(myaction(myaction::INCREMENT));
        sut.dispatch(myaction(myaction::INCREMENT));
        sut.dispatch(myaction(myaction::INCREMENT));
        CHECK(sut.state().value == 5);
        CHECK(sut.revert());
        CHECK(sut.revert());
        CHECK(sut.state().value == 3);
        CHECK(sut.revert());
        CHECK(sut.revert());
        CHECK(sut.revert());
        CHECK(sut.state().value == 0);
        CHECK(!sut.revert());
        CHECK(sut.state().value == 0);
    }
}