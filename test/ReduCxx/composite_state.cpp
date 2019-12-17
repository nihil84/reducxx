#include <ReduCxx/Action.hpp>
#include <ReduCxx/StoreFactory.hpp>
#include "../catch.hpp"
#include <vector>

using namespace ReduCxx;


// interface ReduCxx::Action is only a guideline (supports any type of action)
class MyAction : public Action {
public:
    enum TYPE { INCREMENT, DECREMENT };
    MyAction(TYPE type) : m_type(type) { } // NOLINT(google-explicit-constructor)
    [[nodiscard]] int type() const override { return m_type; }
private:
    TYPE m_type;
};    

struct MyState1 {
    int value;
};

struct MyState2 {
    int value;
    // declared here just to have a member function to use below
    MyState2 reducer(const MyState2& state, const MyAction& action)
    {
        return { state.value +1 };
    }
};

struct MyState3 {
    std::vector<int> ints;
};

MyState1 dummyReducer(const MyState1& state, const MyAction& action)
{
    MyState1 newstate = state;
    switch (action.type()) 
    {
        case MyAction::INCREMENT: newstate.value++; break;
        case MyAction::DECREMENT: newstate.value--; break;
    }
    return newstate;
}

MyState3 nop_reducer(const MyState3& state, const MyAction& action)
{
    return { {3, 4, 5} };
}

int reducerThatThrows(const int& state, const MyAction& action)
{
    if (state == 2) 
    {
        throw "error";
    }
    return state+1;
}

TEST_CASE("composite Store usage example")
{
    MyState2 instance;
    auto memberFun = std::bind(&MyState2::reducer, &instance, std::placeholders::_1, std::placeholders::_2);

    std::function<MyState1(const MyState1&, const MyAction&)> dummy(dummyReducer);

//    auto reducers = Reduce<MyAction>::with(dummy, memberFun, nop_reducer);
//    Store<std::tuple<MyState1, MyState2, MyState3>, MyAction> sut(reducers);
    // short way (leverage template deduction to avoid to write the verbose type of the Store)
    auto sut = StoreFactory<MyAction>::make(
            dummy,          // functor from STL -> MyState1
            memberFun,      // standard binder result -> MyState2
            nop_reducer // plain old C-style function pointer -> MyState3
        // also supports lambdas: another way to use a member function as reducer is the following
        // [&](const MyState2& State, const MyAction& Action)  { return instance.reducer(State, Action); }
    );   

    CHECK(sut.state<MyState1>().value == 0);        // access via State type only works if there is
    CHECK(sut.state<MyState2>().value == 0);        // only one instance of given type per Store
    CHECK(sut.state<MyState3>().ints.size() == 0);  // (pretty common case anyway)
    sut.dispatch( {MyAction::DECREMENT } );
    CHECK(sut.state<0>().value == -1);              // access via index always works:
    CHECK(sut.state<1>().value == 1);               // state indices follows reducers order
    CHECK(sut.state<2>().ints.size() == 3);    
}


TEST_CASE("lambdas")
{
    MyState2 instance;
    std::function<MyState1(const MyState1&, const MyAction&)> dummy(dummyReducer);

    auto sut = StoreFactory<MyAction>::make(
            [&](const MyState2& state, const MyAction& action)  { return instance.reducer(state, action); },
            nop_reducer
    );

    CHECK(sut.state<MyState2>().value == 0);
    CHECK(sut.state<MyState3>().ints.size() == 0);
    sut.dispatch( {MyAction::DECREMENT } );
    CHECK(sut.state<0>().value == 1);
    CHECK(sut.state<1>().ints.size() == 3);
}

SCENARIO("behavioural checks")
{
    GIVEN("a composite Store")
    WHEN("a reducer throws")
    THEN("whole State is not updated")
    {
        // Store dispatch works as a transaction: if any of the reducers throws, the state is not updated (and no
        // subscriber is notified).

        auto sut = StoreFactory<MyAction>::make(dummyReducer, reducerThatThrows);
        CHECK(sut.state<MyState1>().value == 0);
        CHECK(sut.state<int>() == 0);
        sut.dispatch( {MyAction::INCREMENT });
        CHECK(sut.state<MyState1>().value == 1);
        CHECK(sut.state<int>() == 1);
        sut.dispatch( {MyAction::DECREMENT });
        CHECK(sut.state<MyState1>().value == 0);
        CHECK(sut.state<int>() == 2);
        CHECK_THROWS( sut.dispatch({MyAction::INCREMENT }));
        CHECK(sut.state<MyState1>().value == 0);
        CHECK(sut.state<int>() == 2);
    }
}

