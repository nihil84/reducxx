#include <ReduCxx/StoreFactory.hpp>
#include <ReduCxx/Async/ActiveObject.hpp>
#include "../catch.hpp"
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>

using namespace ReduCxx;

struct State {
    int counter = 0;
    bool concurrent = false;
    std::thread::id updated_by;
};

class event { };


SCENARIO("creation and basic features") {

    GIVEN("an async Store")
    WHEN("dispatching event") 
    THEN("it is processed on a separated thread") {
        std::chrono::milliseconds interval(500);
        std::promise<bool> before;
        std::promise<void> after;

        bool concurrent = false;
        // gcc will crash on promise.set_value() if no future has been created yet (shame)
        auto before_future = before.get_future();
        auto after_future = after.get_future();

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            concurrent = before_future.get();           // wait for synchronization [2]
            std::this_thread::sleep_for(interval);
            after.set_value();                          // synchronization [1]
            return {};
        });

        sut.dispatch( {} );
        before.set_value(true);                         // synchronization [2]

        CHECK(after_future.wait_for(interval*2) == std::future_status::ready);
        CHECK(concurrent);                              // synchronization [1]
    }

    GIVEN("an async Store")
    WHEN("the status update throws")
    THEN("the exception is rethrown by the future from dispatch") {
        std::chrono::milliseconds interval(500);
        std::promise<void> before;
        std::promise<void> after;

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            before.get_future().get();     // wait for synchronization [2]
            std::this_thread::sleep_for(interval);
            after.set_value();                          // synchronization [1]
            throw std::exception();
        });

        std::future<void> result = sut.dispatch( {} );
        before.set_value();                      // synchronization [2]

        CHECK(after.get_future().wait_for(interval*2) == std::future_status::ready);
        CHECK_THROWS(result.get());                              // synchronization [1]
    }

    GIVEN("an async Store and a subscribed sync handler")
    WHEN("dispatching event") 
    THEN("the handler runs on the reducer's thread") {
        std::chrono::milliseconds interval(500);
        std::promise<void> before;
        std::promise<void> after;
        ActiveObject<void> activeObject;

        std::thread::id main_thread = std::this_thread::get_id();

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            return {0, true, std::this_thread::get_id()};
        });

        sut.subscribeAsync(activeObject, [&]() {
            before.get_future().wait();
            std::thread::id subscriber_thread = std::this_thread::get_id();
            REQUIRE(main_thread != subscriber_thread);
            REQUIRE(sut.state<State>().updated_by != subscriber_thread);
            after.set_value();
        });

        sut.dispatch( {} ); // not a deadlock!

        before.set_value();
        after.get_future().wait();

        REQUIRE(main_thread != sut.state<State>().updated_by);
    }
}

SCENARIO("active object subsystem") {

    GIVEN("the consumer function of an ActiveObject")
    WHEN("defined and used")
    THEN("it is executed only after the reception of a request") {
        bool executed = false;

        ActiveObject<> sut;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(!executed);

        sut.post( [&]() { executed = true; });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(executed);
    }

    GIVEN("the consumer function of an ActiveObject")
    WHEN("defined and used with a result type")
    THEN("the result value is retrieved via the future") {
        bool executed = false;

        ActiveObject<int> sut;

        CHECK(!executed);
        auto result = sut.post( [&]() { return true; });

        executed = result.get();
        CHECK(executed);
    }

    GIVEN("an ActiveObject")
    WHEN("scheduling a request")
    THEN("it is executed on a separated thread") {
        std::promise<void> before;
        std::promise<void> after;
        bool executed = false;

        auto sut = new ActiveObject<>();

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

    GIVEN("an ActiveObject")
    WHEN("the request handler throws")
    THEN("the control-loop is not interrupted") {
        std::promise<void> mutex;
        bool done = false;

        ActiveObject<> sut;

        std::future<void> result = sut.post( []() {
            throw std::exception();
        });
        CHECK(!done);   // no exception on this thread

        sut.post( [&]() { 
            done = true;
            mutex.set_value();
        });
        CHECK(mutex.get_future().wait_for(std::chrono::milliseconds(500)) == std::future_status::ready);
        CHECK(done);    // ActiveObject is still working

        CHECK(result.valid());
        CHECK_THROWS(result.get());
    }
}

TEST_CASE("asynchronous subscriptions") {

    SECTION("Given an asynchronous subscriber When State is update Then the subscriber is notified") {
        std::promise<void> before;
        std::promise<void> after;
        ActiveObject<void> worker;

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            return {};
        });

        std::shared_ptr<SubscriptionHandle> handle = sut.subscribeAsync(worker, [&]() {
            before.set_value();
            after.get_future().wait();
        });

        sut.dispatch( {} );

        CHECK(before.get_future().wait_for(std::chrono::milliseconds(100)) == std::future_status::ready);
        CHECK(handle->count() == 1);
        after.set_value();

        handle->waitOne();
    }

    SECTION("Given an asynchronous subscriber When the routine throws Then the exception is rethrown on wait") {
        std::promise<void> before;
        std::promise<void> after;
        ActiveObject<void> worker;

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            return {};
        });

        std::shared_ptr<SubscriptionHandle> handle = sut.subscribeAsync(worker, [&]() {
            before.set_value();
            after.get_future().wait();
            throw std::exception();
        });

        sut.dispatch( {} );

        CHECK(before.get_future().wait_for(std::chrono::milliseconds(100)) == std::future_status::ready);
        CHECK(handle->count() == 1);
        after.set_value();

        CHECK_THROWS(handle->waitOne());
    }

    SECTION("Given an asynchronous subscriber When there are many updates Then they can be checked after") {
        static const int WAIT_FOR = 10;
        std::promise<void> done;
        ActiveObject<void> worker;
        std::once_flag set_done_once;

        auto sut = StoreFactory<event>::makeAsync([&](const State& s, const event& e) -> State {
            return {s.counter + 1, s.concurrent, s.updated_by};
        });

        std::shared_ptr<SubscriptionHandle> handle = sut.subscribeAsync(worker, [&]() {
            if (sut.state<State>().counter < WAIT_FOR) return;
            std::call_once(set_done_once, [&]() { done.set_value(); });
        });

        for (int i = 0; i < WAIT_FOR; ++i) {
            sut.dispatch({});
        }

        CHECK(done.get_future().wait_for(std::chrono::milliseconds(300)) == std::future_status::ready);

        CHECK(handle->count() == WAIT_FOR);
        handle->waitAll();
        CHECK(handle->count() == 0);
    }
}
