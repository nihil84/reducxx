#ifndef ASYNC_STORE_HPP
#define ASYNC_STORE_HPP

#include "../store.hpp"
#include "active_object.hpp"

#include <thread>

namespace reducpp {
    template <class S, class A>
    class async_store;
}

template <class S, class A>
class reducpp::async_store : public reducpp::store<S, A> {
public:

    template <class F>
    async_store(const F& reducer) : store<S, A>(reducer) { 
    }

private:

};


#endif // ASYNC_STORE_HPP