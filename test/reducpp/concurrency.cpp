#include <reducpp/store_factory.hpp>
#include <reducpp/async/active_object.hpp>
#include <reducpp/action.hpp>
#include "../doctest.h"

#include <mutex>
#include <chrono>
#include <thread>

using namespace reducpp;


TEST_SUITE_BEGIN("concurrency");

struct state {
    bool concurrent = false;
};

class event {

};


TEST_CASE("creation and basic features" * doctest::may_fail()) {

    GIVEN("an async store") 
    WHEN("dispatching event") 
    THEN("it is processed on a separated thread") {

        std::chrono::milliseconds interval(500);
        std::promise<bool> before;
        std::promise<void> after;

        bool concurrent = false;

        auto sut = store_factory<event>::make_async([&](const state& s, const event& e) -> state {
            concurrent = before.get_future().get();     // wait for synchronization [2]
            std::this_thread::sleep_for(interval);
            after.set_value();                          // synchronization [1]
            return { };
        });

        sut.dispatch( {} );
        before.set_value(true);                    // synchronization [2]

        CHECK(after.get_future().wait_for(interval*2) == std::future_status::ready);
        CHECK(concurrent);                              // synchronization [1]
    }
}

TEST_CASE("active object subsystem") {


    GIVEN("the consumer function of an active_object")
    WHEN("defined and used")
    THEN("it is executed only after the reception of a request") {
        bool executed = false;

        active_object<> sut;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(!executed);

        sut.post( [&]() { executed = true; });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(executed);
    }

    GIVEN("an active_object")
    WHEN("scheduling a request")
    THEN("it is executed on a separated thread") {
        std::promise<void> before;
        std::promise<void> after;
        bool executed = false;

        active_object<>* sut = new active_object<>();

        sut->post( [&]() {
            before.get_future().wait();
            executed = true;
            after.set_value();
        });

        CHECK(!executed); // not a deadlock

        before.set_value();
        after.get_future().wait();
        CHECK(executed);

        delete sut;
    }

    GIVEN("an active_object")
    WHEN("the request handler throws")
    THEN("the control-loop is not interrupted") {
        std::promise<void> mutex;
        bool done = false;

        active_object<> sut;

        std::future<void> result = sut.post( []() {
            throw "an exception";
        });
        CHECK(!done);   // no exception on this thread

        sut.post( [&]() { 
            done = true;
            mutex.set_value();
        });
        CHECK(mutex.get_future().wait_for(std::chrono::milliseconds(500)) == std::future_status::ready);
        CHECK(done);    // active_object is still working

        CHECK(result.valid());
        CHECK_THROWS(result.get());
    }
}

TEST_SUITE_END();