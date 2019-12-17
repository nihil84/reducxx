#ifndef REDUCXX_STORE_FACTORY_HPP
#define REDUCXX_STORE_FACTORY_HPP

#include "Store.hpp"
#include "ReduCxx/Async/AsyncStore.hpp"

namespace ReduCxx {
    template <class A>
    struct StoreFactory;
}

template <class A>
struct ReduCxx::StoreFactory {
    template <class ...Reducers>
    static auto make(const Reducers& ...reducers) {
        using CompositeState = typename Composer<A, Reducers...>::CompositeState;
        return Store<CompositeState, A>(Reduce<A>::with(reducers...));
    }

    template <class ...Reducers>
    static auto makeAsync(const Reducers& ...reducers) {
        using CompositeState = typename Composer<A, Reducers...>::CompositeState;
        return AsyncStore<CompositeState, A>(Reduce<A>::with(reducers...));
    }
};

#endif //REDUCXX_STORE_FACTORY_HPP