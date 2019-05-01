#include <reducpp/store.hpp>
#include <reducpp/action.hpp>
#include "../doctest.h"

using namespace reducpp;

TEST_CASE("basic functionalities")
{

    struct mystate
    {
        int value;
    };

    class myaction : public reducpp::action
    {
      public:
        enum TYPE
        {
            INCREMENT,
            DECREMENT,
            THROW
        };
        myaction(TYPE type) : m_type(type) {}
        int type() const { return m_type; }

      private:
        TYPE m_type;
    };

    store<mystate, myaction> sut([](const mystate &state, const myaction &action) {
        mystate newstate = state;
        switch (action.type())
        {
        case myaction::INCREMENT:
            newstate.value++;
            break;
        case myaction::DECREMENT:
            newstate.value--;
            break;
        case myaction::THROW:
            throw "error";
        }
        return newstate;
    });

    GIVEN("a store")
    WHEN("dispatching an event")
    THEN("the state is updated")
    {
        CHECK(sut.state().value == 0);
        sut.dispatch(myaction(myaction::INCREMENT));
        CHECK(sut.state().value == 1);
        sut.dispatch(myaction(myaction::INCREMENT));
        CHECK(sut.state().value == 2);
        sut.dispatch(myaction(myaction::DECREMENT));
        CHECK(sut.state().value == 1);
    }

    GIVEN("a store with an history of N actions")
    WHEN("reverting")
    THEN("the state is rolled-back")
    {
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

    GIVEN("a store")
    WHEN("the reducer throws")
    THEN("the state is not updated")
    {
        CHECK(sut.state().value == 0);
        sut.dispatch( { myaction::INCREMENT });
        CHECK(sut.state().value == 1);
        CHECK_THROWS(sut.dispatch( { myaction::THROW } ));
        CHECK(sut.state().value == 1);
    }

    GIVEN("a store")
    WHEN("a subscriber throws")
    THEN("state is correctly updated and following subscribers run anyway")
    {
        bool run_anyway = false;
        bool got_exception = false;
        CHECK(sut.state().value == 0);
        sut.subscribe( []() { throw "error"; });
        sut.subscribe( [&]() { run_anyway = true; });
        try 
        {
            sut.dispatch( { myaction::INCREMENT });
        }
        catch (reducpp::store_subscriptions_error& ex) 
        {
            CHECK(ex.errors().size() == 1);
            CHECK(ex.errors()[0].first == 0);
            CHECK_THROWS_AS(std::rethrow_exception(ex.errors()[0].second), const char*);
            got_exception = true;
        }
        CHECK(sut.state().value == 1);
        CHECK(got_exception);
        CHECK(run_anyway);
    }
}
