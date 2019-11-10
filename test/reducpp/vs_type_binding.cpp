#if defined _MSC_VER  // MS Visual Studio


#include <reducpp/reducer_traits.hpp>
#include "../catch.hpp"
#include <iostream>

using namespace std;

template<class F>
struct vs_reducer_traits;

template<class S, class A>
struct vs_reducer_traits<S(const S&, const A&)>
{
    typedef A action_t;
    typedef S state_t;
};

template<typename T, typename S, typename A, typename ...FArgs>
struct vs_reducer_traits<std::_Binder<std::_Unforced, S(__cdecl T::*)(const S&, const A&), FArgs ...>>
: vs_reducer_traits<S(const S&, const A&)>
{};

template<typename T, typename S, typename A, typename ...FArgs>
struct vs_reducer_traits<std::_Binder<std::_Unforced, S(__thiscall T::*)(const S&, const A&), FArgs ...>>
        : vs_reducer_traits<S(const S&, const A&)>
{};


class AClass {
public:
    int do_nothing(const int& s, const float& a) { return 0; }
};

template <class T>
struct Tool {
    using CompositeState = typename vs_reducer_traits<T>::state_t;
};

template <class T>
static Tool<T> get_tool(const T& item) {
    return Tool<T>();
}

TEST_CASE("member function binding") {
    AClass instance;
    auto bound_member = std::bind(&AClass::do_nothing, &instance, std::placeholders::_1, std::placeholders::_2);

    auto tool = get_tool(bound_member);

    CHECK(bound_member(0, 0.3f) == 0);
    CHECK(string(typeid(int).name()) == string(typeid(decltype(tool)::CompositeState).name()));
}


#endif // end of _MSC_VER
