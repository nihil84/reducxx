#include <ReduCxx/Store.hpp>
#include <ReduCxx/Action.hpp>
#include "../catch.hpp"

using namespace ReduCxx;

SCENARIO("basic functionality")
{
    struct MyState
    {
        int value;
    };

    class MyAction : public ReduCxx::Action
    {
      public:
        enum TYPE
        {
            INCREMENT,
            DECREMENT,
            THROW
        };
        MyAction(TYPE type) : m_type(type) {} // NOLINT(google-explicit-constructor)
        [[nodiscard]] int type() const override { return m_type; }

      private:
        TYPE m_type;
    };

    Store<MyState, MyAction> sut([](const MyState &state, const MyAction &action) {
        MyState newstate = state;
        switch (action.type())
        {
        case MyAction::INCREMENT:
            newstate.value++;
            break;
        case MyAction::DECREMENT:
            newstate.value--;
            break;
        case MyAction::THROW:
            throw "error";
        }
        return newstate;
    });

    GIVEN("a Store")
    WHEN("dispatching an event")
    THEN("the State is updated")
    {
        CHECK(sut.state().value == 0);
        sut.dispatch(MyAction(MyAction::INCREMENT));
        CHECK(sut.state().value == 1);
        sut.dispatch(MyAction(MyAction::INCREMENT));
        CHECK(sut.state().value == 2);
        sut.dispatch(MyAction(MyAction::DECREMENT));
        CHECK(sut.state().value == 1);
    }

    GIVEN("a Store with an history of N actions")
    WHEN("reverting")
    THEN("the State is rolled-back")
    {
        CHECK(sut.state().value == 0);
        sut.dispatch(MyAction(MyAction::INCREMENT));
        sut.dispatch(MyAction(MyAction::INCREMENT));
        sut.dispatch(MyAction(MyAction::INCREMENT));
        sut.dispatch(MyAction(MyAction::INCREMENT));
        sut.dispatch(MyAction(MyAction::INCREMENT));
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

    GIVEN("a Store")
    WHEN("the reducer throws")
    THEN("the State is not updated")
    {
        CHECK(sut.state().value == 0);
        sut.dispatch( {MyAction::INCREMENT });
        CHECK(sut.state().value == 1);
        CHECK_THROWS(sut.dispatch( {MyAction::THROW } ));
        CHECK(sut.state().value == 1);
    }

    GIVEN("a Store")
    WHEN("a subscriber throws")
    THEN("State is correctly updated and following subscribers run anyway")
    {
        bool run_anyway = false;
        bool got_exception = false;
        CHECK(sut.state().value == 0);
        sut.subscribe( []() { throw "error"; });
        sut.subscribe( [&]() { run_anyway = true; });
        try 
        {
            sut.dispatch( {MyAction::INCREMENT });
        }
        catch (ReduCxx::StoreSubscriptionsError& ex)
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
