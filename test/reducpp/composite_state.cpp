#include <reducpp/action.hpp>
#include <reducpp/store_factory.hpp>
#include "../doctest.h"
#include <vector>

using namespace reducpp;

TEST_SUITE("composite state") {

// interface reducpp::action is only a guideline (supports any type of action)
class myaction : public action {
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
    // declared here just to have a member function to use below
    mystate2 reducer(const mystate2& state, const myaction& action) 
    {
        return { state.value +1 };
    }
};

struct mystate3 {
    std::vector<int> ints;
};

mystate1 dummy_reducer(const mystate1& state, const myaction& action) 
{
    mystate1 newstate = state;
    switch (action.type()) 
    {
        case myaction::INCREMENT: newstate.value++; break;
        case myaction::DECREMENT: newstate.value--; break;
    }
    return newstate;
}

mystate3 nop_reducer(const mystate3& state, const myaction& action) 
{
    return { {3, 4, 5} };
}

int reducer_that_throws(const int& state, const myaction& action) 
{
    if (state == 2) 
    {
        throw "error";
    }
    return state+1;
}

TEST_CASE("composite store usage example") 
{
    mystate2 instance;
    auto member_fun = std::bind(&mystate2::reducer, &instance, std::placeholders::_1, std::placeholders::_2);

    std::function<mystate1(const mystate1&, const myaction&)> dummy(dummy_reducer);

//    auto reducers = reduce<myaction>::with(dummy, member_fun, nop_reducer);
//    store<std::tuple<mystate1, mystate2, mystate3>, myaction> sut(reducers);
    // short way (leverage template deduction to avoid to write the verbose type of the store)
    auto sut = store_factory<myaction>::make(
        dummy,          // functor from STL -> mystate1
        member_fun,     // standard binder result -> mystate2
        nop_reducer     // plain old C-style function pointer -> mystate3
        // also supports lambdas: another way to use a member function as reducer is the following
        // [&](const mystate2& state, const myaction& action)  { return instance.reducer(state, action); }
    );   

    CHECK(sut.state<mystate1>().value == 0);        // access via state type only works if there is
    CHECK(sut.state<mystate2>().value == 0);        // only one instance of given type per store
    CHECK(sut.state<mystate3>().ints.size() == 0);  // (pretty common case anyway)
    sut.dispatch( { myaction::DECREMENT } );
    CHECK(sut.state<0>().value == -1);              // access via index always works:
    CHECK(sut.state<1>().value == 1);               // state indices follows reducers order
    CHECK(sut.state<2>().ints.size() == 3);    
}


TEST_CASE("lambdas")
{
    mystate2 instance;
    std::function<mystate1(const mystate1&, const myaction&)> dummy(dummy_reducer);

    auto sut = store_factory<myaction>::make(
            [&](const mystate2& state, const myaction& action)  { return instance.reducer(state, action); },
            nop_reducer
    );

    CHECK(sut.state<mystate2>().value == 0);
    CHECK(sut.state<mystate3>().ints.size() == 0);
    sut.dispatch( { myaction::DECREMENT } );
    CHECK(sut.state<0>().value == 1);
    CHECK(sut.state<1>().ints.size() == 3);
}

TEST_CASE("behavioural checks") 
{
    GIVEN("a composite store")
    WHEN("a reducer throws")
    THEN("whole state is not updated")
    {
        // store dispatch works as a transaction: if any of the reducers throws, the state is not updated (and no 
        // subscriber is notified).

        auto sut = store_factory<myaction>::make(dummy_reducer, reducer_that_throws);
        CHECK(sut.state<mystate1>().value == 0);
        CHECK(sut.state<int>() == 0);
        sut.dispatch( { myaction::INCREMENT });
        CHECK(sut.state<mystate1>().value == 1);
        CHECK(sut.state<int>() == 1);
        sut.dispatch( { myaction::DECREMENT });
        CHECK(sut.state<mystate1>().value == 0);
        CHECK(sut.state<int>() == 2);
        CHECK_THROWS( sut.dispatch({ myaction::INCREMENT }));
        CHECK(sut.state<mystate1>().value == 0);
        CHECK(sut.state<int>() == 2);
    }
}

}
