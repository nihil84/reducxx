#ifndef REDUCPP_COMPOSER_HPP
#define REDUCPP_COMPOSER_HPP

#include <tuple>
#include "store.hpp"

namespace reducpp {
    template <typename A, typename S, typename ...States>
    class composer;
}

template <typename A, typename S, typename ...States>
class reducpp::composer {
public:
    composer(const typename store<S, A>::reducer_t& a, const typename store<std::tuple<States...>, A>::reducer_t& b)
        : m_a(a), m_b(b) { }

    std::tuple<S, States...> operator() (const std::tuple<S, States...>& state, const A& action) {
        S substate;
        std::tuple<States...> remainder;
        std::tie(substate, remainder) = state;
        S newsubstate = m_a(substate, action);
        return std::tuple_cat(std::tie(newsubstate), m_b(remainder, action));
    }

private:
    const typename store<S, A>::reducer_t m_a;
    const typename store<std::tuple<States...>, A>::reducer_t m_b;
};


template <typename A>
struct compose {
    template <typename S1, typename S2>
    static reducpp::composer<A, S1, S2> of(const typename reducpp::store<S1, A>::reducer_t& first, 
                                           const typename reducpp::store<S2, A>::reducer_t& second) {
        return reducpp::composer<A, S1, S2>(first, 
                [=](const std::tuple<S2>& s2, const A& a) -> S2 { 
                    return { second(std::get<0>(s2), a)}; 
                });                                    
    }
};


#endif // REDUCPP_COMPOSER_HPP
