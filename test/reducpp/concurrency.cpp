#include <reducpp/store_factory.hpp>
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

TEST_SUITE_END();