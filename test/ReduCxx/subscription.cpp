#include <ReduCxx/Store.hpp>
#include <ReduCxx/Action.hpp>
#include "../catch.hpp"
#include <vector>

using namespace std;
using namespace ReduCxx;


SCENARIO("subscriptions") {

    struct MyState {
        int value;
    };

    class MyAction : public ReduCxx::Action {
    public:
        enum TYPE { INCREMENT, DECREMENT };
        MyAction(TYPE type) : m_type(type) { }
        int type() const override { return m_type; }
    private:
        TYPE m_type;
    };

    Store<MyState, MyAction> sut([](const MyState& state, const MyAction& action) {
        MyState newstate = state;
        switch (action.type()) {
            case MyAction::INCREMENT: newstate.value++; break;
            case MyAction::DECREMENT: newstate.value--; break;
        }
        return newstate;
    });

    GIVEN("a Store and a subscriber")
    WHEN("an event is dispatched") 
    THEN("the subscriber is being called") {
        bool called = false;
        sut.subscribe([&]() { called = true; });

        CHECK(!called);
        sut.dispatch( {MyAction::INCREMENT } );
        CHECK(called);
    }

 
}
