#ifndef REDUCPP_COMPOSER_HPP
#define REDUCPP_COMPOSER_HPP

#include <tuple>
#include <utility>

namespace reducpp {
    template <class A, class ...Reducers>
    class composer;
}

template <class Reducer>
struct reducer_traits;

template <class Functor>
struct reducer_traits : public reducer_traits<decltype(&Functor::operator())> {  };

template <class T, class A, class S>
struct reducer_traits<S(T::*)(const S&, const A&) const> {
    typedef A action_t;
    typedef S ret_t;
    typedef S state_t;
};

template <class A, class ...Reducers>
class reducpp::composer {
public:
    using ReducersTuple = std::tuple<std::decay_t<Reducers> ...>;
    using CompositeState = std::tuple<typename reducer_traits<Reducers>::ret_t ...>;

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
struct reduce {
    template <class ...Reducers>
    static inline reducpp::composer<A, Reducers...> with(Reducers ...reducers) {
        return reducpp::composer<A, Reducers...>(reducers...);
    }
};


#endif // REDUCPP_COMPOSER_HPP
