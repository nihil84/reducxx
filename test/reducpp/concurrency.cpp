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
        std::mutex mutex;
        std::mutex done;

        bool concurrent = false;

        auto sut = store_factory<event>::make_async([&](const state& s, const event& e) -> state {
            std::this_thread::sleep_for(interval);
            concurrent = mutex.try_lock();  // wait for synchronization [2]
            done.unlock();                  // synchronization [1]
            return { };
        });

        done.lock();    // pre-lock the mutex unlocked by the reducer to wait for synchronization [1]
        mutex.lock();   // pre-lock the mutex that the reducer waits for synchronization [2]
        sut.dispatch( {} );
        mutex.unlock(); // synchronization [2]

        done.lock();    // wait for synchronization [1]
        CHECK(concurrent);
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
        std::mutex mutex;
        std::mutex done;
        bool executed = false;

        active_object<> sut;

        mutex.lock();
        done.lock();
        sut.post( [&]() {
            mutex.lock();
            executed = true;
            done.unlock();
        });

        CHECK(!executed); // not a deadlock

        mutex.unlock();
        done.lock();
        CHECK(executed);
    }

    GIVEN("an active_object")
    WHEN("the request handler throws")
    THEN("the control-loop is not interrupted") {
        std::timed_mutex mutex;
        bool done = false;

        active_object<> sut;

        std::future<void> result = sut.post( []() {
            throw "an exception";
        });
        CHECK(!done);   // no exception on this thread
        
        mutex.lock();
        sut.post( [&]() { 
            done = true;
            mutex.unlock();
        });
        CHECK(mutex.try_lock_for(std::chrono::milliseconds(500)));
        CHECK(done);    // active_object is still working

        CHECK(result.valid());
        CHECK_THROWS(result.get());
    }
}

TEST_SUITE_END();