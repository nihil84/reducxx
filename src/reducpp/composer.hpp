#ifndef REDUCPP_COMPOSER_HPP
#define REDUCPP_COMPOSER_HPP

#include <tuple>
#include "store.hpp"

namespace reducpp {
    template <typename A, typename S1, typename S2>
    class composer;
}

template <typename A, typename S1, typename S2>
class reducpp::composer {
public:
    composer(const typename store<S1, A>::reducer_t& a, const typename store<S2, A>::reducer_t& b)
        : m_a(a), m_b(b) { }

    std::tuple<S1, S2> operator() (const std::tuple<S1, S2>& state, const A& action) {
        return std::make_tuple(m_a(std::get<0>(state), action), m_b(std::get<1>(state), action));
    }

private:
    const typename store<S1, A>::reducer_t& m_a;
    const typename store<S2, A>::reducer_t& m_b;
};


#endif // REDUCPP_COMPOSER_HPP