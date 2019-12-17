#ifndef REDUCXX_COMPOSER_HPP
#define REDUCXX_COMPOSER_HPP

#include <tuple>
#include <utility>
#include "reducer_traits.hpp"

namespace ReduCxx
{
    template <class A, class... Reducers>
    class Composer;

    template <class A>
    struct Reduce;
} // namespace ReduCxx

//! @internal
template <class A, class... Reducers>
class ReduCxx::Composer
{
  public:
    using ReducersTuple = std::tuple<std::decay_t<Reducers>...>;
    using CompositeState = std::tuple<typename ReduCxx::_impl::reducer_traits<Reducers>::state_t...>;

    Composer(const Reducers &... reducers)
        : m_reducers(reducers...) {}

    CompositeState operator()(const CompositeState &state, const A &action)
    {
        return apply(state, action, std::index_sequence_for<Reducers...>{});
    }

    template <std::size_t... Is>
    CompositeState apply(const CompositeState &state, const A &action, std::index_sequence<Is...>)
    {
        return {std::get<Is>(m_reducers)(std::get<Is>(state), action)...};
    }

  private:
    const ReducersTuple m_reducers;
};

template <class A>
struct ReduCxx::Reduce
{
    template <class... Reducers>
    static inline Composer<A, Reducers...> with(Reducers... reducers)
    {
        return Composer<A, Reducers...>(reducers...);
    }
};

#endif //REDUCXX_COMPOSER_HPP
