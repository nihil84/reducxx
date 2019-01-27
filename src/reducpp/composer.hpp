#ifndef REDUCPP_COMPOSER_HPP
#define REDUCPP_COMPOSER_HPP

#include <tuple>
#include <utility>
#include "reducer_traits.hpp"

namespace reducpp {
    template <class A, class ...Reducers>
    class composer;

    template <class A>
    struct reduce;
}

template <class A, class ...Reducers>
class reducpp::composer {
public:
    using ReducersTuple = std::tuple<std::decay_t<Reducers> ...>;
    using CompositeState = std::tuple<typename reducpp::_impl::reducer_traits<Reducers>::state_t ...>;

    composer(const Reducers& ...reducers)
        : m_reducers(reducers...) { }

    CompositeState operator() (const CompositeState& state, const A& action) {
        return apply(state, action, std::index_sequence_for<Reducers...>{});
    }

    template <std::size_t ...Is>
    CompositeState apply(const CompositeState& state, const A& action, std::index_sequence<Is...>) {
        return { std::get<Is>(m_reducers)(std::get<Is>(state), action) ... };
    }

private:
    const ReducersTuple m_reducers;
};

template <class A>
struct reducpp::reduce {
    template <class ...Reducers>
    static inline composer<A, Reducers...> with(Reducers ...reducers) {
        return composer<A, Reducers...>(reducers...);
    }
};


#endif // REDUCPP_COMPOSER_HPP
