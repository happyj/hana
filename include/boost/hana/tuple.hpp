/*!
@file
Defines `boost::hana::Tuple`.

@copyright Louis Dionne 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_TUPLE_HPP
#define BOOST_HANA_TUPLE_HPP

#include <boost/hana/fwd/tuple.hpp>

#include <boost/hana/bool.hpp>
#include <boost/hana/comparable.hpp>
#include <boost/hana/constant.hpp>
#include <boost/hana/detail/closure.hpp>
#include <boost/hana/detail/create.hpp>
#include <boost/hana/detail/generate_integer_sequence.hpp>
#include <boost/hana/detail/std/decay.hpp>
#include <boost/hana/detail/std/enable_if.hpp>
#include <boost/hana/detail/std/forward.hpp>
#include <boost/hana/detail/std/integer_sequence.hpp>
#include <boost/hana/detail/std/integral_constant.hpp>
#include <boost/hana/detail/std/move.hpp>
#include <boost/hana/detail/std/remove_reference.hpp>
#include <boost/hana/detail/std/size_t.hpp>
#include <boost/hana/foldable.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/curry.hpp>
#include <boost/hana/functor.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/iterable.hpp>
#include <boost/hana/monad.hpp>
#include <boost/hana/monad_plus.hpp>
#include <boost/hana/searchable.hpp>
#include <boost/hana/type.hpp>

// #include <boost/hana/sequence.hpp>
#include <boost/hana/list.hpp>


namespace boost { namespace hana {
    //////////////////////////////////////////////////////////////////////////
    // tuple, tuple_t and tuple_c
    //////////////////////////////////////////////////////////////////////////
    template <typename ...Xs>
    struct _tuple
        : detail::closure<Xs...>
        , operators::enable_adl
        , operators::Iterable_ops<_tuple<Xs...>>
    {
        using detail::closure<Xs...>::closure; // inherit constructor
        using hana = _tuple;
        using datatype = Tuple;

        static constexpr detail::std::size_t size = sizeof...(Xs);
        static constexpr bool is_empty = sizeof...(Xs) == 0;
    };

    template <typename T, T ...v>
    struct _tuple_c : decltype(tuple(integral_constant<T, v>...)) { };

    template <typename ...T>
    struct _tuple_t : decltype(tuple(type<T>...)) {
        struct _ : _tuple_t { };
    };

    template <>
    struct enabled_operators<Tuple>
        : Comparable, Orderable, Monad, Iterable
    { };

    //////////////////////////////////////////////////////////////////////////
    // make
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct make_impl<Tuple> {
        template <typename ...Xs>
        static constexpr
        _tuple<typename detail::std::decay<Xs>::type...> apply(Xs&& ...xs)
        { return {detail::std::forward<Xs>(xs)...}; }
    };

    //////////////////////////////////////////////////////////////////////////
    // utilities
    //////////////////////////////////////////////////////////////////////////
    namespace tuple_detail {
        template <detail::std::size_t n, typename Xn>
        static constexpr decltype(auto)
        get(detail::element<n, Xn> const& x)
        { return (x.get); }

        template <detail::std::size_t n, typename Xn>
        static constexpr decltype(auto)
        get(detail::element<n, Xn>& x)
        { return (x.get); }

        template <detail::std::size_t n, typename Xn>
        static constexpr Xn
        get(detail::element<n, Xn>&& x)
        { return detail::std::move(x.get); }

        template <typename Xs>
        using size = detail::std::integral_constant<detail::std::size_t,
            detail::std::remove_reference<Xs>::type::size
        >;

        template <bool, typename T>
        using expand = T;
    }

    #define BOOST_HANA_PP_FOR_EACH_REF1(MACRO)                          \
        MACRO(const&)                                                   \
        MACRO(&)                                                        \
        MACRO(&&)                                                       \
    /**/

    #define BOOST_HANA_PP_FOR_EACH_REF2(MACRO)                          \
        MACRO(const&, const&)                                           \
        MACRO(const&, &&    )                                           \
        MACRO(&&    , const&)                                           \
        MACRO(&&    , &&    )                                           \
    /**/

    #define BOOST_HANA_PP_FOR_EACH_REF3(MACRO)                          \
        MACRO(const&, const&, const&)                                   \
        MACRO(const&, const&, &&    )                                   \
        MACRO(const&, &&    , const&)                                   \
        MACRO(const&, &&    , &&    )                                   \
        MACRO(&&    , const&, const&)                                   \
        MACRO(&&    , const&, &&    )                                   \
        MACRO(&&    , &&    , const&)                                   \
        MACRO(&&    , &&    , &&    )                                   \
    /**/

#if 0
    //////////////////////////////////////////////////////////////////////////
    // Comparable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct equal_impl<Tuple, Tuple> {
        template <typename Xs, typename Ys>
        static constexpr decltype(auto) apply(Xs const& xs, Ys const& ys)
        { return Sequence::equal_impl<Tuple, Tuple>::apply(xs, ys); }


        // tuple_t optimizations
        template <typename ...T>
        static constexpr auto apply(_tuple_t<T...>, _tuple_t<T...>)
        { return true_; }

        template <typename ...T, typename ...U>
        static constexpr auto apply(_tuple_t<T...>, _tuple_t<U...>)
        { return false_; }


        // tuple_c optimizations
        template <typename V, V ...v, typename U, U ...u, typename =
            detail::std::enable_if_t<sizeof...(v) == sizeof...(u)>>
        static constexpr auto apply(_tuple_c<V, v...>, _tuple_c<U, u...>) {
            constexpr bool comparisons[] = {(v == u)...};
            return bool_<hana::all_of(comparisons)>;
        }

        template <typename V, V ...v, typename U, U ...u, typename =
            detail::std::enable_if_t<sizeof...(v) != sizeof...(u)>>
        static constexpr auto apply(_tuple_c<V, v...>, _tuple_c<U, u...>, ...)
        { return false_; }
    };
#endif

    //////////////////////////////////////////////////////////////////////////
    // Foldable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct unpack_impl<Tuple> {
        #define BOOST_HANA_PP_UNPACK(REF)                                   \
            template <typename ...Xs, typename F>                           \
            static constexpr decltype(auto)                                 \
            apply(detail::closure_impl<Xs...> REF xs, F&& f) {              \
                return detail::std::forward<F>(f)(                          \
                    static_cast<Xs REF>(xs).get...                          \
                );                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_UNPACK)
        #undef BOOST_HANA_PP_UNPACK

        template <typename ...T, template <typename ...> class F>
        static constexpr auto apply(_tuple_t<T...>, _metafunction<F>)
        { return type<typename F<T...>::type>; }
    };

    template <>
    struct length_impl<Tuple> {
        template <typename Xs>
        static constexpr auto apply(Xs const&)
        { return size_t<Xs::size>; }
    };

    //////////////////////////////////////////////////////////////////////////
    // Iterable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct head_impl<Tuple> {
        template <typename Xs>
        static constexpr decltype(auto) apply(Xs&& xs)
        { return tuple_detail::get<0>(detail::std::forward<Xs>(xs)); }
    };

    template <>
    struct at_impl<Tuple> {
        template <typename N, typename Xs>
        static constexpr decltype(auto) apply(N n, Xs&& xs) {
            constexpr detail::std::size_t index = hana::value(n);
            return tuple_detail::get<index>(detail::std::forward<Xs>(xs));
        }
    };

    template <>
    struct tail_impl<Tuple> {
        #define BOOST_HANA_PP_TAIL(REF)                                     \
            template <typename X, typename ...Xn>                           \
            static constexpr _tuple<typename Xn::get_type...>               \
            apply(detail::closure_impl<X, Xn...> REF xs) {                  \
                return {static_cast<Xn REF>(xs).get...};                    \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_TAIL)
        #undef BOOST_HANA_PP_TAIL
    };

    template <>
    struct is_empty_impl<Tuple> {
        template <typename Xs>
        static constexpr auto apply(Xs const&)
        { return bool_<Xs::is_empty>; }
    };

    template <>
    struct last_impl<Tuple> {
        template <typename Xs>
        static constexpr decltype(auto) apply(Xs&& xs) {
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            return tuple_detail::get<size - 1>(detail::std::forward<Xs>(xs));
        }
    };

    template <>
    struct drop_impl<Tuple> {
        using Size = detail::std::size_t;

        template <Size n, typename Xs, Size ...i>
        static constexpr decltype(auto)
        drop_helper(Xs&& xs, detail::std::index_sequence<i...>) {
            return hana::tuple(tuple_detail::get<n + i>(
                                            detail::std::forward<Xs>(xs))...);
        }

        template <typename N, typename Xs>
        static constexpr decltype(auto) apply(N n_, Xs&& xs) {
            constexpr Size n = hana::value(n_);
            constexpr Size size = tuple_detail::size<Xs>{};
            constexpr Size drop_size = n > size ? size : n;
            return drop_helper<drop_size>(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<size - drop_size>{});
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // Searchable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct find_impl<Tuple>
        : Iterable::find_impl<Tuple>
    { };

    template <>
    struct any_impl<Tuple>
        : Iterable::any_impl<Tuple>
    { };

    //////////////////////////////////////////////////////////////////////////
    // Traversable
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Functor
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct transform_impl<Tuple> {
        #define BOOST_HANA_PP_TRANSFORM(REF)                                \
            template <typename ...Xs, typename F>                           \
            static constexpr decltype(auto)                                 \
            apply(detail::closure_impl<Xs...> REF xs, F&& f)                \
            { return hana::tuple(f(static_cast<Xs REF>(xs).get)...); }      \
                                                                            \
            template <typename X, typename F>                               \
            static constexpr decltype(auto)                                 \
            apply(detail::closure_impl<X> REF xs, F&& f) {                  \
                return hana::tuple(detail::std::forward<F>(f)(              \
                    static_cast<X REF>(xs).get                              \
                ));                                                         \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_TRANSFORM)
        #undef BOOST_HANA_PP_TRANSFORM

        template <typename ...T, template <typename ...> class F>
        static constexpr auto apply(_tuple_t<T...>, _metafunction<F>)
        { return tuple_t<typename F<T>::type...>; }
    };

    template <>
    struct fill_impl<Tuple> {
        template <typename V>
        static constexpr _tuple<> apply(detail::closure_impl<> const&, V&&)
        { return {}; }

        template <typename X, typename ...Xs, typename V>
        static constexpr _tuple<
            typename detail::std::decay<V>::type,
            typename detail::std::decay<
                tuple_detail::expand<!!sizeof(Xs), V>
            >::type...
        > apply(detail::closure_impl<X, Xs...> const&, V&& v)
        { return {((void)sizeof(Xs), v)..., detail::std::forward<V>(v)}; }

        template <typename ...Xs, typename T>
        static constexpr auto apply(_tuple<Xs...> const&, _type<T> const&)
        { return tuple_t<tuple_detail::expand<!!sizeof(Xs), T>...>; }
    };

    //////////////////////////////////////////////////////////////////////////
    // Applicative
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct lift_impl<Tuple> {
        template <typename X>
        static constexpr _tuple<typename detail::std::decay<X>::type> apply(X&& x)
        { return {detail::std::forward<X>(x)}; }
    };

    //////////////////////////////////////////////////////////////////////////
    // Monad
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct flatten_impl<Tuple> {
        using Size = detail::std::size_t;

        template <int Which, Size ...Lengths>
        struct flatten_indices {
            template <typename Array>
            constexpr auto operator()(Array outer) const {
                constexpr Size lengths[] = {Lengths...};
                Array inner = outer;
                for (Size index = 0, i = 0; i < sizeof...(Lengths); ++i) {
                    for (Size j = 0; j < lengths[i]; ++j, ++index) {
                        inner[index] = i;
                        outer[index] = j;
                    }
                }
                return tuple_detail::get<Which>(hana::tuple(outer, inner));
            }
        };

        template <typename Xs, Size ...outer, Size ...inner>
        static constexpr decltype(auto)
        flatten_helper(Xs&& xs, detail::std::index_sequence<outer...>,
                                detail::std::index_sequence<inner...>)
        {
            return hana::tuple(tuple_detail::get<outer>(
                                tuple_detail::get<inner>(
                                    detail::std::forward<Xs>(xs)))...);
        }
        //! @todo
        //! Use `Size` instead of `long long` for the `lengths` array.
        //! The problem is that `hana::sum` does not allow the type of
        //! the elements to be specified and it uses `int_<0>` as a base
        //! value, so this won't work with an unsigned type.
        #define BOOST_HANA_PP_FLATTEN(REF)                                      \
            template <typename ...Xs>                                           \
            static constexpr decltype(auto)                                     \
            apply(detail::closure_impl<Xs...> REF xs) {                         \
                constexpr /* Size */ long long lengths[] = {0,                  \
                    tuple_detail::size<typename Xs::get_type>{}...              \
                };                                                              \
                constexpr Size total_length = hana::sum(lengths);               \
                                                                                \
                using Outer = flatten_indices<0,                                \
                    tuple_detail::size<typename Xs::get_type>{}...              \
                >;                                                              \
                using Inner = flatten_indices<1,                                \
                    tuple_detail::size<typename Xs::get_type>{}...              \
                >;                                                              \
                                                                                \
                return flatten_helper(                                          \
                    static_cast<detail::closure_impl<Xs...> REF>(xs),           \
                    detail::generate_index_sequence<total_length, Outer>{},     \
                    detail::generate_index_sequence<total_length, Inner>{});    \
            }                                                                   \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_FLATTEN)
        #undef BOOST_HANA_PP_FLATTEN

        static constexpr _tuple<> apply(_tuple<> const&) { return {}; }
    };

    //////////////////////////////////////////////////////////////////////////
    // List
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct List::instance<Tuple>
        : List::mcd<Tuple>
    {
        // init
        ///////////////
        template <typename Xs, detail::std::size_t ...n>
        static constexpr decltype(auto)
        init_helper(Xs&& xs, detail::std::index_sequence<n...>) {
            return tuple(tuple_detail::get<n>(detail::std::forward<Xs>(xs))...);
        }

        template <typename Xs>
        static constexpr decltype(auto) init_impl(Xs&& xs) {
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            return init_helper(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<size - 1>{});
        }


        // intersperse
        ///////////////
        template <detail::std::size_t i, typename Z, typename Xs>
        static constexpr Z pick(Z&& z, Xs const&,
            detail::std::false_type /* odd index */)
        { return detail::std::forward<Z>(z); }

        template <detail::std::size_t i, typename Z, typename Xn>
        static constexpr decltype(auto)
        pick(Z const&, detail::element<(i + 1) / 2, Xn> const& x,
            detail::std::true_type /* even index */)
        { return x.get; }

        template <detail::std::size_t i, typename Z, typename Xn>
        static constexpr decltype(auto)
        pick(Z const&, detail::element<(i + 1) / 2, Xn>&& x,
            detail::std::true_type /* even index */)
        { return detail::std::move(x.get); }

        template <typename Xs, typename Z, detail::std::size_t ...i>
        static constexpr decltype(auto)
        intersperse_helper(Xs&& xs, Z&& z, detail::std::index_sequence<i...>) {
            return tuple(
                pick<i>(
                    detail::std::forward<Z>(z),
                    detail::std::forward<Xs>(xs),
                    detail::std::integral_constant<bool, (i % 2 == 0)>{}
                )...
            );
        }

        template <typename Xs, typename Z>
        static constexpr decltype(auto) intersperse_impl(Xs&& xs, Z&& z) {
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            constexpr auto new_seq_size = size == 0 ? 0 : (size * 2) - 1;
            return intersperse_helper(
                detail::std::forward<Xs>(xs),
                detail::std::forward<Z>(z),
                detail::std::make_index_sequence<new_seq_size>{}
            );
        }

        // remove_at
        ///////////////
        template <typename Xs, detail::std::size_t ...before,
                               detail::std::size_t ...after>
        static constexpr decltype(auto)
        remove_at_helper(Xs&& xs, detail::std::index_sequence<before...>,
                                  detail::std::index_sequence<after...>) {
            return tuple(
                tuple_detail::get<before>(detail::std::forward<Xs>(xs))...,
                tuple_detail::get<sizeof...(before) + after + 1>(
                    detail::std::forward<Xs>(xs)
                )...
            );
        }

        template <typename N, typename Xs>
        static constexpr decltype(auto) remove_at_impl(N n, Xs&& xs) {
            constexpr detail::std::size_t index = value(n);
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            return remove_at_helper(
                detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<index>{},
                detail::std::make_index_sequence<size - index - 1>{}
            );
        }


        // reverse
        ///////////////
        template <detail::std::size_t ...n, typename ...Xn>
        static constexpr decltype(auto)
        reverse_impl(detail::closure_impl<detail::element<n, Xn>...>&& xs) {
            return tuple(
                detail::std::move(tuple_detail::get<sizeof...(n) - n - 1>(xs))...
            );
        }

        template <detail::std::size_t ...n, typename ...Xn>
        static constexpr decltype(auto)
        reverse_impl(detail::closure_impl<detail::element<n, Xn>...> const& xs) {
            return tuple(tuple_detail::get<sizeof...(n) - n - 1>(xs)...);
        }

        // slice
        ///////////////
        template <detail::std::size_t from, typename Xs, detail::std::size_t ...i>
        static constexpr decltype(auto)
        slice_helper(Xs&& xs, detail::std::index_sequence<i...>) {
            return tuple(tuple_detail::get<from + i>(detail::std::forward<Xs>(xs))...);
        }

        template <typename Xs, typename From, typename To>
        static constexpr decltype(auto) slice_impl(Xs&& xs, From from_, To to_) {
            constexpr detail::std::size_t from = value(from_), to = value(to_);
            return slice_helper<from>(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<to - from>{}
            );
        }


        // take
        ///////////////
        template <typename Xs, detail::std::size_t ...n>
        static constexpr decltype(auto)
        take_helper(Xs&& xs, detail::std::index_sequence<n...>) {
            return tuple(tuple_detail::get<n>(detail::std::forward<Xs>(xs))...);
        }

        template <typename N, typename Xs>
        static constexpr decltype(auto) take_impl(N n_, Xs&& xs) {
            constexpr detail::std::size_t n = value(n_);
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            return take_helper(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<(n < size ? n : size)>{});
        }


        // unzip
        ///////////////
        #define BOOST_HANA_PP_UNZIP(REF)                                    \
            template <typename ...Xs>                                       \
            static constexpr decltype(auto)                                 \
            unzip_impl(detail::closure_impl<Xs...> REF xs)                  \
            { return zip(static_cast<Xs REF>(xs).get...); }                 \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_UNZIP)
        #undef BOOST_HANA_PP_UNZIP


        // zip_with
        ///////////////
        #define BOOST_HANA_PP_ZIP_WITH1(REF)                                \
            template <typename F, typename ...Xs>                           \
            static constexpr decltype(auto) zip_with_helper(F&& f,          \
                detail::closure_impl<Xs...> REF xs)                         \
            {                                                               \
                return tuple(f(static_cast<Xs REF>(xs).get)...);            \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_ZIP_WITH1)
        #undef BOOST_HANA_PP_ZIP_WITH1

        #define BOOST_HANA_PP_ZIP_WITH2(REF1, REF2)                         \
            template <typename F, typename ...Xs, typename ...Ys>           \
            static constexpr decltype(auto) zip_with_helper(F&& f,          \
                detail::closure_impl<Xs...> REF1 xs,                        \
                detail::closure_impl<Ys...> REF2 ys)                        \
            {                                                               \
                return tuple(                                               \
                    f(static_cast<Xs REF1>(xs).get,                         \
                      static_cast<Ys REF2>(ys).get)...                      \
                );                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF2(BOOST_HANA_PP_ZIP_WITH2)
        #undef BOOST_HANA_PP_ZIP_WITH2

        #define BOOST_HANA_PP_ZIP_WITH3(REF1, REF2, REF3)                         \
            template <typename F, typename ...Xs, typename ...Ys, typename ...Zs> \
            static constexpr decltype(auto) zip_with_helper(F&& f,                \
                detail::closure_impl<Xs...> REF1 xs,                              \
                detail::closure_impl<Ys...> REF2 ys,                              \
                detail::closure_impl<Zs...> REF3 zs)                              \
            {                                                                     \
                return tuple(                                                     \
                    f(static_cast<Xs REF1>(xs).get,                               \
                      static_cast<Ys REF2>(ys).get,                               \
                      static_cast<Zs REF3>(zs).get)...                            \
                );                                                                \
            }                                                                     \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF3(BOOST_HANA_PP_ZIP_WITH3)
        #undef BOOST_HANA_PP_ZIP_WITH3

        template <typename F, typename X1, typename X2, typename X3, typename X4, typename ...Xn>
        static constexpr decltype(auto) zip_with_helper(F&& f, X1&& x1, X2&& x2, X3&& x3, X4&& x4, Xn&& ...xn) {
            return zip_with_helper(apply,
                zip_with_helper(
                    curry<sizeof...(xn) + 4>(detail::std::forward<F>(f)),
                    detail::std::forward<X1>(x1),
                    detail::std::forward<X2>(x2),
                    detail::std::forward<X3>(x3)
                ),
                detail::std::forward<X4>(x4),
                detail::std::forward<Xn>(xn)...
            );
        }

        template <typename F, typename Xs, typename ...Ys>
        static constexpr decltype(auto) zip_with_impl(F&& f, Xs&& xs, Ys&& ...ys) {
            constexpr detail::std::size_t lengths[] = {
                tuple_detail::size<Xs>{},
                tuple_detail::size<Ys>{}...
            };
            constexpr detail::std::size_t zip_length = hana::minimum(lengths);
            return zip_with_helper(detail::std::forward<F>(f),
                take_c<zip_length>(detail::std::forward<Xs>(xs)),
                take_c<zip_length>(detail::std::forward<Ys>(ys))...
            );
        }


        // zip
        ///////////////
        #define BOOST_HANA_PP_ZIP1(REF)                                     \
            template <typename ...Xs>                                       \
            static constexpr _tuple<                                        \
                _tuple<typename Xs::get_type>...                            \
            > zip_helper(detail::closure_impl<Xs...> REF xs) {              \
                return {                                                    \
                    _tuple<typename Xs::get_type>{                          \
                        static_cast<Xs REF>(xs).get                         \
                    }...                                                    \
                };                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_ZIP1)
        #undef BOOST_HANA_PP_ZIP1

        #define BOOST_HANA_PP_ZIP2(REF1, REF2)                              \
            template <typename ...Xs, typename ...Ys>                       \
            static constexpr _tuple<                                        \
                _tuple<typename Xs::get_type,                               \
                       typename Ys::get_type>...                            \
            > zip_helper(detail::closure_impl<Xs...> REF1 xs,               \
                         detail::closure_impl<Ys...> REF2 ys) {             \
                return {                                                    \
                    _tuple<typename Xs::get_type,                           \
                           typename Ys::get_type>{                          \
                        static_cast<Xs REF1>(xs).get,                       \
                        static_cast<Ys REF2>(ys).get                        \
                    }...                                                    \
                };                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF2(BOOST_HANA_PP_ZIP2)
        #undef BOOST_HANA_PP_ZIP2

        #define BOOST_HANA_PP_ZIP3(REF1, REF2, REF3)                    \
            template <typename ...Xs, typename ...Ys, typename ...Zs>   \
            static constexpr _tuple<                                    \
                _tuple<typename Xs::get_type,                           \
                       typename Ys::get_type,                           \
                       typename Zs::get_type>...                        \
            > zip_helper(detail::closure_impl<Xs...> REF1 xs,           \
                         detail::closure_impl<Ys...> REF2 ys,           \
                         detail::closure_impl<Zs...> REF3 zs) {         \
                return {                                                \
                    _tuple<typename Xs::get_type,                       \
                           typename Ys::get_type,                       \
                           typename Zs::get_type>{                      \
                        static_cast<Xs REF1>(xs).get,                   \
                        static_cast<Ys REF2>(ys).get,                   \
                        static_cast<Zs REF3>(zs).get                    \
                    }...                                                \
                };                                                      \
            }                                                           \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF3(BOOST_HANA_PP_ZIP3)
        #undef BOOST_HANA_PP_ZIP3

        template <typename X1, typename X2, typename X3, typename X4, typename ...Xn>
        static constexpr decltype(auto)
        zip_helper(X1&& x1, X2&& x2, X3&& x3, X4&& x4, Xn&& ...xn) {
            return zip_with_helper(concat,
                zip_helper(
                    detail::std::forward<X1>(x1),
                    detail::std::forward<X2>(x2),
                    detail::std::forward<X3>(x3)
                ),
                zip_helper(
                    detail::std::forward<X4>(x4),
                    detail::std::forward<Xn>(xn)...
                )
            );
        }

        template <typename Xs, typename ...Ys>
        static constexpr decltype(auto) zip_impl(Xs&& xs, Ys&& ...ys) {
            constexpr detail::std::size_t lengths[] = {
                tuple_detail::size<Xs>{},
                tuple_detail::size<Ys>{}...
            };
            constexpr detail::std::size_t zip_length = hana::minimum(lengths);
            return zip_helper(
                take_c<zip_length>(detail::std::forward<Xs>(xs)),
                take_c<zip_length>(detail::std::forward<Ys>(ys))...
            );
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // MonadPlus
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct concat_impl<Tuple> {
        #define BOOST_HANA_PP_CONCAT(REF1, REF2)                            \
            template <typename ...Xs, typename ...Ys>                       \
            static constexpr _tuple<                                        \
                typename Xs::get_type..., typename Ys::get_type...          \
            > apply(detail::closure_impl<Xs...> REF1 xs,                    \
                    detail::closure_impl<Ys...> REF2 ys)                    \
            {                                                               \
                return {static_cast<Xs REF1>(xs).get...,                    \
                        static_cast<Ys REF2>(ys).get...};                   \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF2(BOOST_HANA_PP_CONCAT)
        #undef BOOST_HANA_PP_CONCAT
    };

    template <>
    struct nil_impl<Tuple> {
        static constexpr _tuple<> apply()
        { return {}; }
    };

    template <>
    struct repeat_impl<Tuple> {
        template <typename X>
        static constexpr _tuple<>
        repeat_helper(X&&, detail::std::index_sequence<>)
        { return {}; }

        template <typename X, detail::std::size_t i, detail::std::size_t ...j>
        static constexpr _tuple<
            typename detail::std::decay<X>::type,
            typename detail::std::decay<tuple_detail::expand<!!j, X>>::type...
        > repeat_helper(X&& x, detail::std::index_sequence<i, j...>)
        { return {((void)j, x)..., detail::std::forward<X>(x)}; }

        template <typename T, detail::std::size_t ...i>
        static constexpr auto
        repeat_helper(_type<T>, detail::std::index_sequence<i...>)
        { return tuple_t<tuple_detail::expand<!!i, T>...>; }

        template <typename N, typename X>
        static constexpr decltype(auto) apply(N n_, X&& x) {
            constexpr detail::std::size_t n = hana::value(n_);
            return repeat_helper(detail::std::forward<X>(x),
                                 detail::std::make_index_sequence<n>{});
        }
    };

    template <>
    struct cycle_impl<Tuple> {
        template <typename N, typename Xs>
        static constexpr decltype(auto) apply(N n, Xs&& xs)
        { return hana::flatten(hana::repeat<Tuple>(n, detail::std::forward<Xs>(xs))); }
    };

    template <>
    struct prepend_impl<Tuple> {
        #define BOOST_HANA_PP_PREPEND(REF)                                      \
            template <typename X, typename ...Xs>                               \
            static constexpr _tuple<                                            \
                typename detail::std::decay<X>::type, typename Xs::get_type...  \
            > apply(X&& x, detail::closure_impl<Xs...> REF xs) {                \
                return {                                                        \
                    detail::std::forward<X>(x), static_cast<Xs REF>(xs).get...  \
                };                                                              \
            }                                                                   \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_PREPEND)
        #undef BOOST_HANA_PP_PREPEND
    };

    template <>
    struct append_impl<Tuple> {
        #define BOOST_HANA_PP_APPEND(REF)                                       \
            template <typename ...Xs, typename X>                               \
            static constexpr _tuple<                                            \
                typename Xs::get_type..., typename detail::std::decay<X>::type  \
            > apply(detail::closure_impl<Xs...> REF xs, X&& x) {                \
                return {                                                        \
                    static_cast<Xs REF>(xs).get..., detail::std::forward<X>(x)  \
                };                                                              \
            }                                                                   \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_APPEND)
        #undef BOOST_HANA_PP_APPEND
    };

#if 0
    //////////////////////////////////////////////////////////////////////////
    // Sequence
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct init_impl<Tuple> {
        template <typename Xs, detail::std::size_t ...n>
        static constexpr decltype(auto)
        init_helper(Xs&& xs, detail::std::index_sequence<n...>) {
            return hana::tuple(detail::tuple_get<n>(
                    detail::std::forward<Xs>(xs))...);
        }

        template <typename Xs>
        static constexpr decltype(auto) apply(Xs&& xs) {
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            return init_helper(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<size - 1>{});
        }
    };

    template <>
    struct intersperse_impl<Tuple> {
        template <detail::std::size_t i, typename Z, typename Xs>
        static constexpr Z pick(Z&& z, Xs const&,
            detail::std::false_type /* odd index */)
        { return detail::std::forward<Z>(z); }

        template <detail::std::size_t i, typename Z, typename Xn>
        static constexpr decltype(auto)
        pick(Z const&, detail::element<(i + 1) / 2, Xn> const& x,
            detail::std::true_type /* even index */)
        { return x.get; }

        template <detail::std::size_t i, typename Z, typename Xn>
        static constexpr decltype(auto)
        pick(Z const&, detail::element<(i + 1) / 2, Xn>&& x,
            detail::std::true_type /* even index */)
        { return detail::std::move(x.get); }

        template <typename Xs, typename Z, detail::std::size_t ...i>
        static constexpr decltype(auto)
        helper(Xs&& xs, Z&& z, detail::std::index_sequence<i...>) {
            return hana::tuple(
                pick<i>(
                    detail::std::forward<Z>(z),
                    detail::std::forward<Xs>(xs),
                    detail::std::integral_constant<bool, (i % 2 == 0)>{}
                )...
            );
        }

        template <typename Xs, typename Z>
        static constexpr decltype(auto) apply(Xs&& xs, Z&& z) {
            constexpr detail::std::size_t size = tuple_detail::size<Xs>{};
            constexpr auto new_seq_size = size == 0 ? 0 : (size * 2) - 1;
            return helper(detail::std::forward<Xs>(xs),
                          detail::std::forward<Z>(z),
                          detail::std::make_index_sequence<new_seq_size>{});
        }
    };

    template <>
    struct remove_at_impl<Tuple> {
        using Size = detail::std::size_t;

        template <typename Xs, Size ...before, Size ...after>
        static constexpr decltype(auto)
        remove_at_helper(Xs&& xs, detail::std::index_sequence<before...>,
                                  detail::std::index_sequence<after...>)
        {
            return hana::tuple(
                detail::tuple_get<before>(
                    detail::std::forward<Xs>(xs)
                )...,
                detail::tuple_get<sizeof...(before) + after + 1>(
                    detail::std::forward<Xs>(xs)
                )...
            );
        }

        template <typename N, typename Xs>
        static constexpr decltype(auto) apply(N n, Xs&& xs) {
            constexpr Size index = hana::value(n);
            constexpr Size size = tuple_detail::size<Xs>{};
            return remove_at_helper(detail::std::forward<Xs>(xs),
                          detail::std::make_index_sequence<index>{},
                          detail::std::make_index_sequence<size - index - 1>{});
        }
    };

    template <>
    struct reverse_impl<Tuple> {
        template <detail::std::size_t ...n, typename ...Xn>
        static constexpr decltype(auto)
        apply(detail::closure_impl<detail::element<n, Xn>...>&& xs) {
            return hana::tuple(detail::std::move(
                    detail::tuple_get<sizeof...(n) - n - 1>(xs))...);
        }

        template <detail::std::size_t ...n, typename ...Xn>
        static constexpr decltype(auto)
        apply(detail::closure_impl<detail::element<n, Xn>...> const& xs) {
            return hana::tuple(detail::tuple_get<sizeof...(n) - n - 1>(xs)...);
        }
    };

    template <>
    struct slice_impl<Tuple> {
        template <detail::std::size_t from, typename Xs, detail::std::size_t ...i>
        static constexpr decltype(auto)
        helper(Xs&& xs, detail::std::index_sequence<i...>) {
            return hana::tuple(detail::tuple_get<from + i>(
                                    detail::std::forward<Xs>(xs))...);
        }

        template <typename Xs, typename From, typename To>
        static constexpr decltype(auto) apply(Xs&& xs, From, To) {
            constexpr detail::std::size_t from = hana::value2<From>(),
                                          to = hana::value2<To>();
            return helper<from>(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<to - from>{}
            );
        }
    };

    template <>
    struct take_impl<Tuple> {
        template <typename Xs, detail::std::size_t ...n>
        static constexpr decltype(auto)
        helper(Xs&& xs, detail::std::index_sequence<n...>) {
            return hana::tuple(detail::tuple_get<n>(
                                        detail::std::forward<Xs>(xs))...);
        }

        template <typename N, typename Xs>
        static constexpr decltype(auto) apply(N, Xs&& xs) {
            constexpr detail::std::size_t n = hana::value2<N>(),
                                          size = tuple_detail::size<Xs>{};
            return helper(detail::std::forward<Xs>(xs),
                detail::std::make_index_sequence<(n < size ? n : size)>{});
        }
    };

    template <>
    struct unzip_impl<Tuple> {
        #define BOOST_HANA_PP_UNZIP(REF)                                    \
            template <typename ...Xs>                                       \
            static constexpr decltype(auto)                                 \
            apply(detail::closure_impl<Xs...> REF xs)                       \
            { return hana::zip(static_cast<Xs REF>(xs).get...); }           \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_UNZIP)
        #undef BOOST_HANA_PP_UNZIP
    };

    template <>
    struct zip_with_impl<Tuple> {
        #define BOOST_HANA_PP_ZIP_WITH1(REF)                                \
            template <typename F, typename ...Xs>                           \
            static constexpr decltype(auto) zip_with_helper(F&& f,          \
                detail::closure_impl<Xs...> REF xs)                         \
            { return hana::tuple(f(static_cast<Xs REF>(xs).get)...); }      \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_ZIP_WITH1)
        #undef BOOST_HANA_PP_ZIP_WITH1

        #define BOOST_HANA_PP_ZIP_WITH2(REF1, REF2)                         \
            template <typename F, typename ...Xs, typename ...Ys>           \
            static constexpr decltype(auto) zip_with_helper(F&& f,          \
                detail::closure_impl<Xs...> REF1 xs,                        \
                detail::closure_impl<Ys...> REF2 ys)                        \
            {                                                               \
                return hana::tuple(                                         \
                    f(static_cast<Xs REF1>(xs).get,                         \
                      static_cast<Ys REF2>(ys).get)...                      \
                );                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF2(BOOST_HANA_PP_ZIP_WITH2)
        #undef BOOST_HANA_PP_ZIP_WITH2

        #define BOOST_HANA_PP_ZIP_WITH3(REF1, REF2, REF3)                         \
            template <typename F, typename ...Xs, typename ...Ys, typename ...Zs> \
            static constexpr decltype(auto) zip_with_helper(F&& f,                \
                detail::closure_impl<Xs...> REF1 xs,                              \
                detail::closure_impl<Ys...> REF2 ys,                              \
                detail::closure_impl<Zs...> REF3 zs)                              \
            {                                                                     \
                return hana::tuple(                                               \
                    f(static_cast<Xs REF1>(xs).get,                               \
                      static_cast<Ys REF2>(ys).get,                               \
                      static_cast<Zs REF3>(zs).get)...                            \
                );                                                                \
            }                                                                     \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF3(BOOST_HANA_PP_ZIP_WITH3)
        #undef BOOST_HANA_PP_ZIP_WITH3

        template <typename F, typename X1, typename X2, typename X3, typename X4, typename ...Xn>
        static constexpr decltype(auto)
        zip_with_helper(F&& f, X1&& x1, X2&& x2, X3&& x3, X4&& x4, Xn&& ...xn) {
            return zip_with_helper(hana::apply,
                zip_with_helper(
                    curry<sizeof...(xn) + 4>(detail::std::forward<F>(f)),
                    detail::std::forward<X1>(x1),
                    detail::std::forward<X2>(x2),
                    detail::std::forward<X3>(x3)
                ),
                detail::std::forward<X4>(x4),
                detail::std::forward<Xn>(xn)...
            );
        }

        template <typename F, typename Xs, typename ...Ys>
        static constexpr decltype(auto) apply(F&& f, Xs&& xs, Ys&& ...ys) {
            constexpr detail::std::size_t lengths[] = {
                tuple_detail::size<Xs>{}, tuple_detail::size<Ys>{}...
            };
            constexpr detail::std::size_t zip_length = hana::minimum(lengths);
            return zip_with_helper(detail::std::forward<F>(f),
                take_c<zip_length>(detail::std::forward<Xs>(xs)),
                take_c<zip_length>(detail::std::forward<Ys>(ys))...
            );
        }
    };

    template <>
    struct zip_impl<Tuple> {
        #define BOOST_HANA_PP_ZIP1(REF)                                     \
            template <typename ...Xs>                                       \
            static constexpr _tuple<                                        \
                _tuple<typename Xs::get_type>...                            \
            > zip_helper(detail::closure_impl<Xs...> REF xs) {              \
                return {                                                    \
                    _tuple<typename Xs::get_type>{                          \
                        static_cast<Xs REF>(xs).get                         \
                    }...                                                    \
                };                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF1(BOOST_HANA_PP_ZIP1)
        #undef BOOST_HANA_PP_ZIP1

        #define BOOST_HANA_PP_ZIP2(REF1, REF2)                              \
            template <typename ...Xs, typename ...Ys>                       \
            static constexpr _tuple<                                        \
                _tuple<typename Xs::get_type,                               \
                       typename Ys::get_type>...                            \
            > zip_helper(detail::closure_impl<Xs...> REF1 xs,               \
                         detail::closure_impl<Ys...> REF2 ys) {             \
                return {                                                    \
                    _tuple<typename Xs::get_type,                           \
                           typename Ys::get_type>{                          \
                        static_cast<Xs REF1>(xs).get,                       \
                        static_cast<Ys REF2>(ys).get                        \
                    }...                                                    \
                };                                                          \
            }                                                               \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF2(BOOST_HANA_PP_ZIP2)
        #undef BOOST_HANA_PP_ZIP2

        #define BOOST_HANA_PP_ZIP3(REF1, REF2, REF3)                    \
            template <typename ...Xs, typename ...Ys, typename ...Zs>   \
            static constexpr _tuple<                                    \
                _tuple<typename Xs::get_type,                           \
                       typename Ys::get_type,                           \
                       typename Zs::get_type>...                        \
            > zip_helper(detail::closure_impl<Xs...> REF1 xs,           \
                         detail::closure_impl<Ys...> REF2 ys,           \
                         detail::closure_impl<Zs...> REF3 zs) {         \
                return {                                                \
                    _tuple<typename Xs::get_type,                       \
                           typename Ys::get_type,                       \
                           typename Zs::get_type>{                      \
                        static_cast<Xs REF1>(xs).get,                   \
                        static_cast<Ys REF2>(ys).get,                   \
                        static_cast<Zs REF3>(zs).get                    \
                    }...                                                \
                };                                                      \
            }                                                           \
        /**/
        BOOST_HANA_PP_FOR_EACH_REF3(BOOST_HANA_PP_ZIP3)
        #undef BOOST_HANA_PP_ZIP3

        template <typename X1, typename X2, typename X3, typename X4, typename ...Xn>
        static constexpr decltype(auto)
        zip_helper(X1&& x1, X2&& x2, X3&& x3, X4&& x4, Xn&& ...xn) {
            return hana::zip_with(concat,
                zip_helper(
                    detail::std::forward<X1>(x1),
                    detail::std::forward<X2>(x2),
                    detail::std::forward<X3>(x3)
                ),
                zip_helper(
                    detail::std::forward<X4>(x4),
                    detail::std::forward<Xn>(xn)...
                )
            );
        }

        template <typename Xs, typename ...Ys>
        static constexpr decltype(auto) apply(Xs&& xs, Ys&& ...ys) {
            constexpr detail::std::size_t lengths[] = {
                tuple_detail::size<Xs>{}, tuple_detail::size<Ys>{}...
            };
            constexpr detail::std::size_t zip_length = hana::minimum(lengths);
            return zip_helper(
                take_c<zip_length>(detail::std::forward<Xs>(xs)),
                take_c<zip_length>(detail::std::forward<Ys>(ys))...
            );
        }
    };
#endif

#undef BOOST_HANA_PP_FOR_EACH_REF1
#undef BOOST_HANA_PP_FOR_EACH_REF2
#undef BOOST_HANA_PP_FOR_EACH_REF3
}} // end namespace boost::hana

#endif // !BOOST_HANA_TUPLE_HPP
