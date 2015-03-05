/*
 * STL API extracted from libc++ for use in-kernel.
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.libc++ for details.
 */

#pragma once

#include <sys/types.h>

#ifdef KERNEL
/* placement new support */
inline void* operator new  (size_t, void* __p) noexcept {return __p;}
inline void* operator new[](size_t, void* __p) noexcept {return __p;}
#endif

namespace ttl {

struct __nat {
    __nat() = delete;
    __nat(const __nat&) = delete;
    __nat& operator=(const __nat&) = delete;
    ~__nat() = delete;
};

/* std::nullptr_t */
typedef decltype(nullptr) nullptr_t;
    
/* std::integral_constant */
struct __two {char __lx[2];};
template <class _Tp, _Tp __v> struct integral_constant {
    static constexpr const _Tp value = __v;
    typedef _Tp value_type;
    typedef integral_constant type;
    constexpr operator value_type() const noexcept {return value;}
};

template <class _Tp, _Tp __v> constexpr const _Tp integral_constant<_Tp, __v>::value;

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

/* std:true_type, std::false_type */
typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

/* std::conditional */
template <bool _Bp, class _If, class _Then> struct conditional {typedef _If type;};
template <class _If, class _Then> struct conditional<false, _If, _Then> {typedef _Then type;};

/* std::is_lvalue_reference */
template <class _Tp> struct is_lvalue_reference : public false_type {};
template <class _Tp> struct is_lvalue_reference<_Tp&> : public true_type {};
template <class _Tp> struct is_rvalue_reference : public false_type {};
template <class _Tp> struct is_rvalue_reference<_Tp&&> : public true_type {};
    
/* std::remove_reference */
template <class _Tp> struct remove_reference {typedef _Tp type;};
template <class _Tp> struct remove_reference<_Tp&> {typedef _Tp type;};
template <class _Tp> struct remove_reference<_Tp&&> {typedef _Tp type;};

/* std::move */
template <class _Tp> inline typename remove_reference<_Tp>::type&& move(_Tp&& __t) noexcept  {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
}

/* std::forward */
template <class _Tp> inline _Tp&& forward(typename ttl::remove_reference<_Tp>::type& __t) noexcept { return static_cast<_Tp&&>(__t); }
template <class _Tp> inline _Tp&& forward(typename ttl::remove_reference<_Tp>::type&& __t) noexcept {
    static_assert(!ttl::is_lvalue_reference<_Tp>::value, "Can not forward an rvalue as an lvalue.");
    return static_cast<_Tp&&>(__t);
}

/* std::enable_if */
template <bool, class _Tp = void> struct enable_if {};
template <class _Tp> struct enable_if<true, _Tp> {typedef _Tp type;};

/* std::is_array */
template <class _Tp> struct is_array : public false_type {};
template <class _Tp> struct is_array<_Tp[]> : public true_type {};
template <class _Tp, size_t _Np> struct is_array<_Tp[_Np]> : public true_type {};

/* std::is_const */
template <class _Tp> struct is_const : public false_type {};
template <class _Tp> struct is_const<_Tp const> : public true_type {};

/* std::is_volatile */
template <class _Tp> struct is_volatile : public false_type {};
template <class _Tp> struct is_volatile<_Tp volatile> : public true_type {};

/* std::remove_const */
template <class _Tp> struct remove_const {typedef _Tp type;};
template <class _Tp> struct remove_const<const _Tp> {typedef _Tp type;};

/* std::remove_volatile */
template <class _Tp> struct remove_volatile {typedef _Tp type;};
template <class _Tp> struct remove_volatile<volatile _Tp> {typedef _Tp type;};

/* std::is_union */
template <class _Tp> struct is_union
        : public integral_constant<bool, __is_union(_Tp)> {};

/* std::is_class */
template <class _Tp> struct is_class
        : public integral_constant<bool, __is_class(_Tp)> {};

    
/* std::is_abstract */
namespace __is_abstract_imp {
    template <class _Tp> char __test(_Tp (*)[1]);
    template <class _Tp> __two __test(...);
}

template <class _Tp, bool = is_class<_Tp>::value> struct __libcpp_abstract : public integral_constant<bool, sizeof(__is_abstract_imp::__test<_Tp>(0)) != 1> {};
template <class _Tp> struct __libcpp_abstract<_Tp, false> : public false_type {};
template <class _Tp> struct is_abstract : public __libcpp_abstract<_Tp> {};

/* std::is_same */
template <class _Tp, class _Up> struct is_same : public false_type {};
template <class _Tp> struct is_same<_Tp, _Tp> : public true_type {};

/* std::remove_cv */
template <class _Tp> struct remove_cv
    {typedef typename remove_volatile<typename remove_const<_Tp>::type>::type type;};

/* std::is_void */
template <class _Tp> struct __libcpp_is_void : public false_type {};
template <> struct __libcpp_is_void<void> : public true_type {};

template <class _Tp> struct is_void
        : public __libcpp_is_void<typename remove_cv<_Tp>::type> {};

/* std::is_reference */
template <class _Tp> struct is_reference : public false_type {};
template <class _Tp> struct is_reference<_Tp&> : public true_type {};
template <class _Tp> struct is_reference<_Tp&&> : public true_type {};
    
/* std::__is_nullptr_t */
template <class _Tp> struct __is_nullptr_t_impl : public false_type {};
template <> struct __is_nullptr_t_impl<nullptr_t> : public true_type {};
template <class _Tp> struct __is_nullptr_t : public __is_nullptr_t_impl<typename remove_cv<_Tp>::type> {};
    
/* std::is_function */
namespace __libcpp_is_function_imp
{
    template <class _Tp> char  __test(_Tp*);
    template <class _Tp> __two __test(...);
    template <class _Tp> _Tp&  __source();
}

/* std::is_floating_point */
template <class _Tp> struct __libcpp_is_floating_point : public false_type {};
template <> struct __libcpp_is_floating_point<float> : public true_type {};
template <> struct __libcpp_is_floating_point<double> : public true_type {};
template <> struct __libcpp_is_floating_point<long double> : public true_type {};

template <class _Tp> struct is_floating_point : public __libcpp_is_floating_point<typename remove_cv<_Tp>::type> {};

/* std::is_integral */
template <class _Tp> struct __libcpp_is_integral : public false_type {};
template <> struct __libcpp_is_integral<bool> : public true_type {};
template <> struct __libcpp_is_integral<char> : public true_type {};
template <> struct __libcpp_is_integral<signed char> : public true_type {};
template <> struct __libcpp_is_integral<unsigned char> : public true_type {};
template <> struct __libcpp_is_integral<wchar_t> : public true_type {};

template <> struct __libcpp_is_integral<char16_t> : public true_type {};
template <> struct __libcpp_is_integral<char32_t> : public true_type {};

template <> struct __libcpp_is_integral<short> : public true_type {};
template <> struct __libcpp_is_integral<unsigned short> : public true_type {};
template <> struct __libcpp_is_integral<int> : public true_type {};
template <> struct __libcpp_is_integral<unsigned int> : public true_type {};
template <> struct __libcpp_is_integral<long> : public true_type {};
template <> struct __libcpp_is_integral<unsigned long> : public true_type {};
template <> struct __libcpp_is_integral<long long> : public true_type {};
template <> struct __libcpp_is_integral<unsigned long long> : public true_type {};

template <> struct __libcpp_is_integral<__int128_t> : public true_type {};
template <> struct __libcpp_is_integral<__uint128_t> : public true_type {};

template <class _Tp> struct is_integral : public __libcpp_is_integral<typename remove_cv<_Tp>::type> {};

/* std::is_enum */
template <class _Tp> struct is_enum
    : public integral_constant<bool, __is_enum(_Tp)> {};

/* std::is_arithmetic */
template <class _Tp> struct is_arithmetic
    : public integral_constant<bool, is_integral<_Tp>::value ||
                                     is_floating_point<_Tp>::value> {};

/* std::is_pointer */
template <class _Tp> struct __libcpp_is_pointer : public false_type {};
template <class _Tp> struct __libcpp_is_pointer<_Tp*> : public true_type {};
template <class _Tp> struct is_pointer : public __libcpp_is_pointer<typename remove_cv<_Tp>::type> {};

/* std::is_member_function_pointer */
template <class _MP, bool _IsMemberFuctionPtr, bool _IsMemberObjectPtr> struct __member_pointer_traits_imp {};

namespace __libcpp_is_member_function_pointer_imp {
    template <typename _Tp>
    char __test(typename ttl::__member_pointer_traits_imp<_Tp, true, false>::_FnType *);

    template <typename>
    ttl::__two __test(...);
};

template <class _Tp> struct __libcpp_is_member_function_pointer
    : public integral_constant<bool, sizeof(__libcpp_is_member_function_pointer_imp::__test<_Tp>(nullptr)) == 1> {};

template <class _Tp> struct is_member_function_pointer
    : public __libcpp_is_member_function_pointer<typename remove_cv<_Tp>::type> {};


/* std::is_member_pointer */
template <class _Tp> struct __libcpp_is_member_pointer : public false_type {};
template <class _Tp, class _Up> struct __libcpp_is_member_pointer<_Tp _Up::*> : public true_type {};

template <class _Tp> struct is_member_pointer : public __libcpp_is_member_pointer<typename remove_cv<_Tp>::type> {};


/* std::is_member_object_pointer */
template <class _Tp> struct is_member_object_pointer
    : public integral_constant<bool, is_member_pointer<_Tp>::value &&
                                    !is_member_function_pointer<_Tp>::value> {};

/* std::is_scalar */
template <class _Tp> struct is_scalar
: public integral_constant<bool, is_arithmetic<_Tp>::value ||
                                 is_member_pointer<_Tp>::value ||
                                 is_pointer<_Tp>::value ||
                                 __is_nullptr_t<_Tp>::value ||
                                 is_enum<_Tp>::value > {};

template <> struct is_scalar<nullptr_t> : public true_type {};


/* std::add_rvalue_reference */
template <class _Tp> struct add_rvalue_reference {typedef _Tp&& type;};
template <> struct add_rvalue_reference<void> {typedef void type;};
template <> struct add_rvalue_reference<const void> {typedef const void type;};
template <> struct add_rvalue_reference<volatile void> {typedef volatile void type;};
template <> struct add_rvalue_reference<const volatile void> {typedef const volatile void type;};

/* std::add_lvalue_reference */
template <class _Tp> struct add_lvalue_reference {typedef _Tp& type;};
template <class _Tp> struct add_lvalue_reference<_Tp&> {typedef _Tp& type;};
template <> struct add_lvalue_reference<void> {typedef void type;};
template <> struct add_lvalue_reference<const void> {typedef const void type;};
template <> struct add_lvalue_reference<volatile void> {typedef volatile void type;};
template <> struct add_lvalue_reference<const volatile void> {typedef const volatile void type;};

/* std::declval */
template <class _Tp> typename add_rvalue_reference<_Tp>::type declval() noexcept;


/* std::is_function */
template <class _Tp, bool = is_class<_Tp>::value ||
        is_union<_Tp>::value ||
        is_void<_Tp>::value  ||
        is_reference<_Tp>::value ||
        __is_nullptr_t<_Tp>::value >
struct __libcpp_is_function
        : public integral_constant<bool, sizeof(__libcpp_is_function_imp::__test<_Tp>(__libcpp_is_function_imp::__source<_Tp>())) == 1>
{};
template <class _Tp> struct __libcpp_is_function<_Tp, true> : public false_type {};

template <class _Tp> struct is_function : public __libcpp_is_function<_Tp> {};

/* std::add_const */
template <class _Tp, bool = is_reference<_Tp>::value ||
                            is_function<_Tp>::value ||
                            is_const<_Tp>::value >
struct __add_const {typedef _Tp type;};

template <class _Tp> struct __add_const<_Tp, false> {typedef const _Tp type;};

template <class _Tp> struct add_const {typedef typename __add_const<_Tp>::type type;};

/* std::remove_extent */
template <class _Tp> struct remove_extent {typedef _Tp type;};
template <class _Tp> struct remove_extent<_Tp[]> {typedef _Tp type;};
template <class _Tp, size_t _Np> struct remove_extent<_Tp[_Np]> {typedef _Tp type;};

/* std::remove_all_extents */
template <class _Tp> struct remove_all_extents {typedef _Tp type;};
template <class _Tp> struct remove_all_extents<_Tp[]> {typedef typename remove_all_extents<_Tp>::type type;};
template <class _Tp, size_t _Np> struct remove_all_extents<_Tp[_Np]> {typedef typename remove_all_extents<_Tp>::type type;};

/* std::add_pointer */
template <class _Tp> struct add_pointer {typedef typename remove_reference<_Tp>::type* type;};

/* std::decay */
template <class _Tp> struct decay {
private:
    typedef typename remove_reference<_Tp>::type _Up;
public:
    typedef typename conditional
                     <
                         is_array<_Up>::value,
                         typename remove_extent<_Up>::type*,
                         typename conditional
                         <
                              is_function<_Up>::value,
                              typename add_pointer<_Up>::type,
                              typename remove_cv<_Up>::type
                         >::type
                     >::type type;
};

/* std::is_base_of */
template <class _Bp, class _Dp> struct is_base_of : public integral_constant<bool, __is_base_of(_Bp, _Dp)> {};

/* std::is_convertible */
namespace __is_convertible_imp {
    template <class _Tp> char  __test(_Tp);
    template <class _Tp> __two __test(...);
    template <class _Tp> _Tp&& __source();
    
    template <class _Tp, bool _IsArray =    is_array<_Tp>::value,
    bool _IsFunction = is_function<_Tp>::value,
    bool _IsVoid =     is_void<_Tp>::value>
    struct __is_array_function_or_void                          {enum {value = 0};};
    template <class _Tp> struct __is_array_function_or_void<_Tp, true, false, false> {enum {value = 1};};
    template <class _Tp> struct __is_array_function_or_void<_Tp, false, true, false> {enum {value = 2};};
    template <class _Tp> struct __is_array_function_or_void<_Tp, false, false, true> {enum {value = 3};};
}

template <class _Tp,
unsigned = __is_convertible_imp::__is_array_function_or_void<typename remove_reference<_Tp>::type>::value>
struct __is_convertible_check
{
    static const size_t __v = 0;
};

template <class _Tp>
struct __is_convertible_check<_Tp, 0>
{
    static const size_t __v = sizeof(_Tp);
};

template <class _T1, class _T2,
unsigned _T1_is_array_function_or_void = __is_convertible_imp::__is_array_function_or_void<_T1>::value,
unsigned _T2_is_array_function_or_void = __is_convertible_imp::__is_array_function_or_void<_T2>::value>
struct __is_convertible_ : public integral_constant<bool, sizeof(__is_convertible_imp::__test<_T2>(__is_convertible_imp::__source<_T1>())) == 1>
{};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 1, 0> : false_type {};

template <class _T1> struct __is_convertible_<_T1, const _T1&, 1, 0> : true_type {};
template <class _T1> struct __is_convertible_<_T1, _T1&&, 1, 0> : true_type {};
template <class _T1> struct __is_convertible_<_T1, const _T1&&, 1, 0> : true_type {};
template <class _T1> struct __is_convertible_<_T1, volatile _T1&&, 1, 0> : true_type {};
template <class _T1> struct __is_convertible_<_T1, const volatile _T1&&, 1, 0> : true_type {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2*, 1, 0>
: public integral_constant<bool, __is_convertible_<typename remove_all_extents<_T1>::type*, _T2*>::value> {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2* const, 1, 0>
: public integral_constant<bool, __is_convertible_<typename remove_all_extents<_T1>::type*, _T2*const>::value> {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2* volatile, 1, 0>
: public integral_constant<bool, __is_convertible_<typename remove_all_extents<_T1>::type*, _T2*volatile>::value> {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2* const volatile, 1, 0>
: public integral_constant<bool, __is_convertible_<typename remove_all_extents<_T1>::type*, _T2*const volatile>::value> {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 2, 0>                : public false_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1&&, 2, 0>               : public true_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1&, 2, 0>               : public true_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1*, 2, 0>               : public true_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1*const, 2, 0>          : public true_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1*volatile, 2, 0>       : public true_type {};
template <class _T1>            struct __is_convertible_<_T1, _T1*const volatile, 2, 0> : public true_type {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 3, 0> : public false_type {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 0, 1> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 1, 1> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 2, 1> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 3, 1> : public false_type {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 0, 2> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 1, 2> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 2, 2> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 3, 2> : public false_type {};

template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 0, 3> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 1, 3> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 2, 3> : public false_type {};
template <class _T1, class _T2> struct __is_convertible_<_T1, _T2, 3, 3> : public true_type {};

template <class _T1, class _T2> struct is_convertible : public __is_convertible_<_T1, _T2>
{
    static const size_t __complete_check1 = __is_convertible_check<_T1>::__v;
    static const size_t __complete_check2 = __is_convertible_check<_T2>::__v;
};

/* std::array (partial implementation) */
template <class _Tp, size_t _Size> struct array {
    typedef array __self;
    typedef _Tp value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef size_t size_type;

    value_type __elems_[_Size > 0 ? _Size : 1];

    iterator begin() noexcept {return iterator(__elems_);}
    const_iterator begin() const noexcept {return const_iterator(__elems_);}
    iterator end() noexcept {return iterator(__elems_ + _Size);}
    const_iterator end() const noexcept {return const_iterator(__elems_ + _Size);}

    const_iterator cbegin() const noexcept {return begin();}
    const_iterator cend() const noexcept {return end();}

    constexpr size_type size() const noexcept {return _Size;}
    constexpr size_type max_size() const noexcept {return _Size;}
    constexpr bool empty() const noexcept {return _Size == 0;}


    reference operator[](size_type __n) {return __elems_[__n];}
    const_reference operator[](size_type __n) const {return __elems_[__n];}
    reference at(size_type __n);
                                  const_reference at(size_type __n) const;

    reference front() {return __elems_[0];}
    const_reference front() const {return __elems_[0];}
    reference back() {return __elems_[_Size > 0 ? _Size-1 : 0];}
    const_reference back() const {return __elems_[_Size > 0 ? _Size-1 : 0];}

    value_type* data() noexcept {return __elems_;}
    const value_type* data() const noexcept {return __elems_;}
};

/* std::iterator_tag */
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

/* std::initializer_list (partial implementation) */
template<class _Ep> class initializer_list {
    const _Ep* __begin_;
    size_t __size_;

    initializer_list(const _Ep* __b, size_t __s) noexcept : __begin_(__b), __size_(__s) {}

public:
    typedef _Ep value_type;
    typedef const _Ep& reference;
    typedef const _Ep& const_reference;
    typedef size_t size_type;
    
    typedef const _Ep* iterator;
    typedef const _Ep* const_iterator;

    initializer_list() noexcept : __begin_(nullptr), __size_(0) {}
    size_t size() const noexcept {return __size_;}
    const _Ep* begin() const noexcept {return __begin_;}
    const _Ep* end() const noexcept {return __begin_ + __size_;}
};

/* std::common_type */
template <class ..._Tp> struct common_type;

template <class _Tp>
struct common_type<_Tp>
{
    typedef typename decay<_Tp>::type type;
};

template <class _Tp, class _Up>
struct common_type<_Tp, _Up>
{
private:
    static _Tp&& __t();
    static _Up&& __u();
    static bool __f();
public:
    typedef typename decay<decltype(__f() ? __t() : __u())>::type type;
};

template <class _Tp, class _Up, class ..._Vp>
struct common_type<_Tp, _Up, _Vp...>
{
    typedef typename common_type<typename common_type<_Tp, _Up>::type, _Vp...>::type type;
};

/* std:is_*_constructible */
struct __any {
    __any(...);
};

template<typename, typename _Tp> struct __select_2nd { typedef _Tp type; };
    template <class _Tp, class _Arg> typename __select_2nd<decltype((declval<_Tp>() = declval<_Arg>())), true_type>::type
    __is_assignable_test(_Tp&&, _Arg&&);

    
namespace __is_construct {
    struct __nat {};
}

template <class _Tp, class ..._Args>
typename __select_2nd<decltype(move(_Tp(declval<_Args>()...))), true_type>::type
__is_constructible_test(_Tp&&, _Args&& ...);

template <class ..._Args>
false_type
__is_constructible_test(__any, _Args&& ...);

template <bool, class _Tp, class... _Args>
struct __libcpp_is_constructible
: public common_type
<
decltype(__is_constructible_test(declval<_Tp>(), declval<_Args>()...))
>::type
{};

template <class _Rp, class... _A1, class... _A2>
struct __libcpp_is_constructible<false, _Rp(_A1...), _A2...>
: public false_type
{};

template <class _Tp>
struct __libcpp_is_constructible<true, _Tp>
: public is_scalar<_Tp>
{};

template <class _Tp>
struct __is_constructible_ref
{
    true_type static __lxx(_Tp);
    false_type static __lxx(...);
};

template <class _Tp, class _A0>
struct __libcpp_is_constructible<true, _Tp, _A0>
: public common_type
<
decltype(__is_constructible_ref<_Tp>::__lxx(declval<_A0>()))
>::type
{};

template <class _Tp, class _A0, class ..._Args>
struct __libcpp_is_constructible<true, _Tp, _A0, _Args...>
: public false_type
{};

template <bool, class _Tp, class... _Args>
struct __is_constructible_void_check
: public __libcpp_is_constructible<is_scalar<_Tp>::value || is_reference<_Tp>::value,
_Tp, _Args...>
{};

template <class _Tp, class... _Args>
struct __is_constructible_void_check<true, _Tp, _Args...>
: public false_type
{};

template <class ..._Args> struct __contains_void;

template <> struct __contains_void<> : false_type {};

template <class _A0, class ..._Args>
struct __contains_void<_A0, _Args...>
{
    static const bool value = is_void<_A0>::value ||
    __contains_void<_Args...>::value;
};

template <class _Tp, class... _Args>
struct is_constructible
: public __is_constructible_void_check<__contains_void<_Tp, _Args...>::value
|| is_abstract<_Tp>::value,
_Tp, _Args...>
{};

template <class _Ap, size_t _Np>
struct __libcpp_is_constructible<false, _Ap[_Np]>
: public is_constructible<typename remove_all_extents<_Ap>::type>
{};

template <class _Ap, size_t _Np, class ..._Args>
struct __libcpp_is_constructible<false, _Ap[_Np], _Args...>
: public false_type
{};

template <class _Ap, class ..._Args>
struct __libcpp_is_constructible<false, _Ap[], _Args...>
: public false_type
{};
template <class _Tp>
struct is_default_constructible
: public is_constructible<_Tp>
{};

template <class _Tp>
struct is_copy_constructible
: public is_constructible<_Tp,
typename add_lvalue_reference<typename add_const<_Tp>::type>::type> {};

template <class _Tp>
struct is_move_constructible : public is_constructible<_Tp, typename add_rvalue_reference<_Tp>::type>
{};


template <bool, bool, class _Tp, class... _Args> struct __libcpp_is_nothrow_constructible;

template <class _Tp, class... _Args> struct __libcpp_is_nothrow_constructible< true, false, _Tp, _Args...> : public integral_constant<bool, noexcept(_Tp(declval<_Args>()...))>
{
};

template <class _Tp> void __implicit_conversion_to(_Tp) noexcept { }

template <class _Tp, class _Arg> struct __libcpp_is_nothrow_constructible< true, true, _Tp, _Arg> : public integral_constant<bool, noexcept(__implicit_conversion_to<_Tp>(declval<_Arg>()))>
{
};

template <class _Tp, bool _IsReference, class... _Args> struct __libcpp_is_nothrow_constructible< false, _IsReference, _Tp, _Args...> : public false_type
{
};

template <class _Tp, class... _Args> struct is_nothrow_constructible : __libcpp_is_nothrow_constructible<is_constructible<_Tp, _Args...>::value, is_reference<_Tp>::value, _Tp, _Args...>
{
};

template <class _Tp, size_t _Ns> struct is_nothrow_constructible<_Tp[_Ns]> : __libcpp_is_nothrow_constructible<is_constructible<_Tp>::value, is_reference<_Tp>::value, _Tp>
{
};


template <class _Tp> struct is_nothrow_copy_constructible : public is_nothrow_constructible<_Tp, typename add_lvalue_reference<typename add_const<_Tp>::type>::type> {};

template <class _Tp> struct is_nothrow_move_constructible : public is_nothrow_constructible<_Tp, typename add_rvalue_reference<_Tp>::type> {};

template <class _Tp> struct is_nothrow_default_constructible : public is_nothrow_constructible<_Tp> {};

/* _member_pointer_traits */
template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...), true, false>
{
    typedef _Class _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...), true, false>
{
    typedef _Class _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const, true, false>
{
    typedef _Class const _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const, true, false>
{
    typedef _Class const _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) volatile, true, false>
{
    typedef _Class volatile _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) volatile, true, false>
{
    typedef _Class volatile _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const volatile, true, false>
{
    typedef _Class const volatile _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const volatile, true, false>
{
    typedef _Class const volatile _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) &, true, false>
{
    typedef _Class& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) &, true, false>
{
    typedef _Class& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const&, true, false>
{
    typedef _Class const& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const&, true, false>
{
    typedef _Class const& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) volatile&, true, false>
{
    typedef _Class volatile& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) volatile&, true, false>
{
    typedef _Class volatile& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const volatile&, true, false>
{
    typedef _Class const volatile& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const volatile&, true, false>
{
    typedef _Class const volatile& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) &&, true, false>
{
    typedef _Class&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) &&, true, false>
{
    typedef _Class&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const&&, true, false>
{
    typedef _Class const&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const&&, true, false>
{
    typedef _Class const&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) volatile&&, true, false>
{
    typedef _Class volatile&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) volatile&&, true, false>
{
    typedef _Class volatile&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param...) const volatile&&, true, false>
{
    typedef _Class const volatile&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param...);
};

template <class _Rp, class _Class, class ..._Param>
struct __member_pointer_traits_imp<_Rp (_Class::*)(_Param..., ...) const volatile&&, true, false>
{
    typedef _Class const volatile&& _ClassType;
    typedef _Rp _ReturnType;
    typedef _Rp (_FnType) (_Param..., ...);
};

template <class _Rp, class _Class>
struct __member_pointer_traits_imp<_Rp _Class::*, false, true>
{
    typedef _Class _ClassType;
    typedef _Rp _ReturnType;
};

template <class _MP>
struct __member_pointer_traits
    : public __member_pointer_traits_imp<typename remove_cv<_MP>::type,
                    is_member_function_pointer<_MP>::value,
                    is_member_object_pointer<_MP>::value>
{



};

/* _member_pointer_traits */
template <class ..._Tp> struct __check_complete;

template <>
struct __check_complete<>
{
};

template <class _Hp, class _T0, class ..._Tp>
struct __check_complete<_Hp, _T0, _Tp...>
    : private __check_complete<_Hp>,
      private __check_complete<_T0, _Tp...>
{
};

template <class _Hp>
struct __check_complete<_Hp, _Hp>
    : private __check_complete<_Hp>
{
};

template <class _Tp>
struct __check_complete<_Tp>
{
    static_assert(sizeof(_Tp) > 0, "Type must be complete.");
};

template <class _Tp>
struct __check_complete<_Tp&>
    : private __check_complete<_Tp>
{
};

template <class _Tp>
struct __check_complete<_Tp&&>
    : private __check_complete<_Tp>
{
};

template <class _Rp, class ..._Param>
struct __check_complete<_Rp (*)(_Param...)>
    : private __check_complete<_Rp>
{
};

template <class ..._Param>
struct __check_complete<void (*)(_Param...)>
{
};

template <class _Rp, class ..._Param>
struct __check_complete<_Rp (_Param...)>
    : private __check_complete<_Rp>
{
};

template <class ..._Param>
struct __check_complete<void (_Param...)>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...)>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) volatile>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const volatile>
    : private __check_complete<_Class>
{
};



template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) &>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) volatile&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const volatile&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) &&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const&&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) volatile&&>
    : private __check_complete<_Class>
{
};

template <class _Rp, class _Class, class ..._Param>
struct __check_complete<_Rp (_Class::*)(_Param...) const volatile&&>
    : private __check_complete<_Class>
{
};



template <class _Rp, class _Class>
struct __check_complete<_Rp _Class::*>
    : private __check_complete<_Class>
{
};


/* std::result_of */
template <class _Callable> class result_of;
    
template <class ..._Args> auto __invoke(__any, _Args&& ...__args) -> __nat;

template <class _Fp, class _A0, class ..._Args,
            class = typename enable_if
            <
                is_member_function_pointer<typename remove_reference<_Fp>::type>::value &&
                is_base_of<typename remove_reference<typename __member_pointer_traits<typename remove_reference<_Fp>::type>::_ClassType>::type,
                           typename remove_reference<_A0>::type>::value
            >::type
         >

auto
__invoke(_Fp&& __f, _A0&& __a0, _Args&& ...__args)
    -> decltype((forward<_A0>(__a0).*__f)(forward<_Args>(__args)...));

template <class _Fp, class _A0, class ..._Args,
            class = typename enable_if
            <
                is_member_function_pointer<typename remove_reference<_Fp>::type>::value &&
                !is_base_of<typename remove_reference<typename __member_pointer_traits<typename remove_reference<_Fp>::type>::_ClassType>::type,
                           typename remove_reference<_A0>::type>::value
            >::type
         >

auto
__invoke(_Fp&& __f, _A0&& __a0, _Args&& ...__args)
    -> decltype(((*forward<_A0>(__a0)).*__f)(forward<_Args>(__args)...));



template <class _Fp, class _A0,
            class = typename enable_if
            <
                is_member_object_pointer<typename remove_reference<_Fp>::type>::value &&
                is_base_of<typename __member_pointer_traits<typename remove_reference<_Fp>::type>::_ClassType,
                           typename remove_reference<_A0>::type>::value
            >::type
         >

auto
__invoke(_Fp&& __f, _A0&& __a0)
    -> decltype(forward<_A0>(__a0).*__f);

template <class _Fp, class _A0,
            class = typename enable_if
            <
                is_member_object_pointer<typename remove_reference<_Fp>::type>::value &&
                !is_base_of<typename __member_pointer_traits<typename remove_reference<_Fp>::type>::_ClassType,
                           typename remove_reference<_A0>::type>::value
            >::type
         >

auto
__invoke(_Fp&& __f, _A0&& __a0)
    -> decltype((*forward<_A0>(__a0)).*__f);



template <class _Fp, class ..._Args>

auto
__invoke(_Fp&& __f, _Args&& ...__args)
    -> decltype(forward<_Fp>(__f)(forward<_Args>(__args)...));



template <class _Fp, class ..._Args>
struct __invokable_imp
    : private __check_complete<_Fp>
{
    typedef decltype(
            __invoke(declval<_Fp>(), declval<_Args>()...)
                    ) type;
    static const bool value = !is_same<type, __nat>::value;
};

template <class _Fp, class ..._Args>
struct __invokable
    : public integral_constant<bool,
          __invokable_imp<_Fp, _Args...>::value>
{
};



template <bool _Invokable, class _Fp, class ..._Args>
struct __invoke_of_imp
{
};

template <class _Fp, class ..._Args>
struct __invoke_of_imp<true, _Fp, _Args...>
{
    typedef typename __invokable_imp<_Fp, _Args...>::type type;
};

template <class _Fp, class ..._Args>
struct __invoke_of
    : public __invoke_of_imp<__invokable<_Fp, _Args...>::value, _Fp, _Args...>
{
};

template <class _Fp, class ..._Args>
class result_of<_Fp(_Args...)>
    : public __invoke_of<_Fp, _Args...>
{
};


/* std::is_pod */
template <class _Tp> struct is_pod : public integral_constant<bool, __is_pod(_Tp)> {};
template <class _Tp> struct is_literal_type : public integral_constant<bool, __is_literal(_Tp)> {};

/* std::is_standard_layout */
template <class _Tp> struct is_standard_layout : public integral_constant<bool, __is_standard_layout(_Tp)> {};

/* std::is_trivially_copyable */
template <class _Tp> struct is_trivially_copyable : public integral_constant<bool, __is_trivially_copyable(_Tp)> {};

/* std::is_trivial */
template <class _Tp> struct is_trivial : public integral_constant<bool, __is_trivial(_Tp)> {};

} /* namespace ttl */