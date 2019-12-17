#ifndef STORE_FACTORY_HPP
#define STORE_FACTORY_HPP

#include "store.hpp"
#include "async/async_store.hpp"

namespace ReduCxx {
    template <class A>
    struct store_factory;
}

template <class A>
struct ReduCxx::store_factory {
    template <class ...Reducers>
    static auto make(const Reducers& ...reducers) {
        using CompositeState = typename composer<A, Reducers...>::CompositeState;
        return store<CompositeState, A>(reduce<A>::with(reducers...));
    }

    template <class ...Reducers>
    static auto make_async(const Reducers& ...reducers) {
        using CompositeState = typename composer<A, Reducers...>::CompositeState;
        return async_store<CompositeState, A>(reduce<A>::with(reducers...));
    }
};

#endif // STORE_FACTORY_HPP 