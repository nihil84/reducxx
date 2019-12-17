#ifndef REDUCXX_REDUCER_TRAITS_HPP
#define REDUCXX_REDUCER_TRAITS_HPP

#include <functional>

namespace ReduCxx::_impl {

template<class F>
struct ReducerTraits;

template<class S, class A>
struct ReducerTraits<S(const S&, const A&)>
{
    typedef A Action_t;
    typedef S State_t;
};
 
// function pointer
template<class S, class A>
struct ReducerTraits<S(*)(const S&, const A&)> : public ReducerTraits<S(const S&, const A&)> {};

// member function pointer
template <class T, class S, class A>
struct ReducerTraits<S(T::*)(const S&, const A&)> : public ReducerTraits<S(const S&, const A&)> {};
 
// const member function pointer
template <class T, class S, class A>
struct ReducerTraits<S(T::*)(const S&, const A&) const> : public ReducerTraits<S(const S&, const A&)> {};

// functor
template<class F>
struct ReducerTraits : public ReducerTraits<decltype(&F::operator())> {};

// reference to functor
template<class F>
struct ReducerTraits<F&> : public ReducerTraits<F> {};
 
// rvalue of functor
template<class F>
struct ReducerTraits<F&&> : public ReducerTraits<F> {};

// std::bind for object methods
template<typename T, typename S, typename A, typename ...FArgs>
#if defined _LIBCPP_VERSION  // libc++ (Clang)
struct ReducerTraits<std::__bind<S (T::*)(const S&, const A&), FArgs ...>>
#elif defined _GLIBCXX_RELEASE  // glibc++ (GNU C++ >= 7.1)
struct ReducerTraits<std::_Bind<S(T::*(FArgs ...))(const S&, const A&)>>
#elif defined __GLIBCXX__  // glibc++ (GNU C++)
struct ReducerTraits<std::_Bind<std::_Mem_fn<S(T::*)(const S&, const A&)>(FArgs ...)>>
#elif defined _MSC_VER  // MS Visual Studio
struct ReducerTraits<std::_Binder<std::_Unforced, S(__thiscall T::*)(const S&, const A&), FArgs ...>>
#else
#error "Unsupported C++ compiler / standard library"
#endif
  : ReducerTraits<S(const S&, const A&)>
{};


}


#endif //REDUCXX_REDUCER_TRAITS_HPP
