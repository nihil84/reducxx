
namespace reducpp {
namespace _impl {


template<class F>
struct reducer_traits;
 
// function pointer
template<class S, class A>
struct reducer_traits<S(*)(const S&, const A&)> : public reducer_traits<S(const S&, const A&)> {};

// member function pointer
template <class T, class S, class A>
struct reducer_traits<S(T::*)(const S&, const A&)> : public reducer_traits<S(const S&, const A&)> {};
 
// const member function pointer
template <class T, class S, class A>
struct reducer_traits<S(T::*)(const S&, const A&) const> : public reducer_traits<S(const S&, const A&)> {};

// functor
template<class F>
struct reducer_traits : public reducer_traits<decltype(&F::operator())> {};

// reference to functor
template<class F>
struct reducer_traits<F&> : public reducer_traits<F> {};
 
// rvalue of functor
template<class F>
struct reducer_traits<F&&> : public reducer_traits<F> {};
 
template<class S, class A>
struct reducer_traits<S(const S&, const A&)>
{
    typedef A action_t;
    typedef S state_t;
};

}}
