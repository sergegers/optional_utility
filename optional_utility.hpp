﻿#ifndef OPTIONAL_UTILITY_HPP
#define OPTIONAL_UTILITY_HPP

#include <functional>
#include <type_traits>
#include <utility>
#include <boost/optional.hpp>

namespace optional_utility
{
    template <typename Adaptor, typename Optional>
    class optional_view;

    namespace optional_utility_detail
    {
        template <typename T>
        struct is_optional
            : std::false_type
        {
        };

        template <typename T>
        struct is_optional<boost::optional<T>>
            : std::true_type
        {
        };

        template <typename Adaptor, typename Optional>
        struct is_optional<optional_view<Adaptor, Optional>>
            : std::true_type
        {
        };
    }
    
    template <typename T>
    inline typename boost::optional<T>::reference_const_type value(boost::optional<T> const& op)
    {
        return op.value();
    }

    template <typename T>
    inline typename boost::optional<T>::reference_type value(boost::optional<T>& op)
    {
        return op.value();
    }

    template <typename T>
    inline typename boost::optional<T>::reference_type_of_temporary_wrapper value(boost::optional<T>&& op)
    {
        return std::move(op.value());
    }

    template <typename T, typename U>
    inline typename boost::optional<T>::value_type value_or(boost::optional<T> const& op, U&& v)
    {
        return op.value_or(std::forward<U>(v));
    }

    template <typename T, typename U>
    inline typename boost::optional<T>::value_type value_or(boost::optional<T>&& op, U&& v)
    {
        return std::move(op.value_or(std::forward<U>(v)));
    }

    template <typename T, typename F>
    inline typename boost::optional<T>::value_type value_or_eval(boost::optional<T> const& op, F&& f)
    {
        return op.value_or_eval(std::forward<F>(f));
    }

    template <typename T, typename F>
    inline typename boost::optional<T>::value_type value_or_eval(boost::optional<T>&& op, F&& f)
    {
        return std::move(op.value_or_eval(std::forward<F>(f)));
    }

    template <typename Exception, typename T, typename ...Args>
    typename boost::optional<T>::reference_type value_or_throw(boost::optional<T>& op, Args&&... args)
    {
        if (!op) {
            throw Exception{std::forward<Args>(args)...};
        }
        return op.value();
    }

    template <typename Exception, typename T, typename ...Args>
    typename boost::optional<T>::reference_const_type value_or_throw(boost::optional<T> const& op, Args&&... args)
    {
        if (!op) {
            throw Exception{std::forward<Args>(args)...};
        }
        return op.value();
    }

    template <typename Exception, typename T, typename ...Args>
    auto value_or_throw(boost::optional<T>&& op, Args&&... args)
        -> typename boost::optional<T>::reference_type_of_temporary_wrapper
    {
        if (!op) {
            throw Exception{std::forward<Args>(args)...};
        }
        return std::move(op.value());
    }
    
    template <typename T>
    class optional_wrapper
    {
        boost::optional<T> optional;
    public:
        explicit optional_wrapper(boost::optional<T> op)
            : optional{std::move(op)}
        {
        }

        boost::optional<T> evaluate() const&
        {
            return optional;
        }

        boost::optional<T> evaluate() &&
        {
            return std::move(optional);
        }
    };

    template <typename Adaptor, typename Optional>
    class optional_view
    {
        boost::optional<Adaptor> adaptor;
        Optional optional;
    public:
        optional_view(boost::none_t)
            : adaptor{boost::none}, optional{boost::none}
        {
        }

        optional_view(Adaptor adapt, Optional op)
            : adaptor{std::move(adapt)}, optional{std::move(op)}
        {
        }

        template <typename T>
        operator boost::optional<T> () const&
        {
            return adaptor ? adaptor.get()(optional.evaluate()) : boost::none;
        }

        template <typename T>
        operator boost::optional<T>() &&
        {
            return adaptor ? std::move(adaptor.get()(optional.evaluate())) : boost::none;
        }

        auto evaluate() const&
        {
            return adaptor ? adaptor.get()(optional.evaluate()) : boost::none;
        }

        auto evaluate() &&
        {
            return adaptor ? adaptor.get()(optional.evaluate()) : boost::none;
        }
    };

    template <typename F>
    class flat_mapped_t
    {
        F f;
    public:
        explicit flat_mapped_t(F f)
            : f(std::move(f))
        {
        }

        template <typename T, typename R = typename std::result_of<F(T)>::type>
        R operator()(boost::optional<T> const& op) const
        {
            if (!op) {
                return boost::none;
            }
            return f(op.get());
        }

        inline boost::none_t operator()(boost::none_t const&) const noexcept
        {
            return boost::none;
        }
    };

    template <typename F>
    inline flat_mapped_t<F> flat_mapped(F&& f)
    {
        return flat_mapped_t<F>(std::forward<F>(f));
    }

    template <typename R, typename T, typename ...Params, typename ...Args>
    inline auto flat_mapped(R(T::*mem_fun)(Params...) const, Args&&... args)
    {
        // be careful to referenced object lifetime
        return flat_mapped([&, mem_fun](T const& t) { return (t.*mem_fun)(std::forward<Args>(args)...); });
    }

    template <typename F>
    class mapped_t
    {
        F f;
    public:
        explicit mapped_t(F f)
            : f(std::move(f))
        {
        }

        template <typename T, typename R = typename std::result_of<F(T)>::type>
        boost::optional<R> operator()(boost::optional<T> const& op) const
        {
            static_assert(!std::is_void<R>::value, "Function must not return void type");
            if (!op) {
                return boost::none;
            }
            return boost::make_optional(f(op.get()));
        }

        inline boost::none_t operator()(boost::none_t const&) const noexcept
        {
            return boost::none;
        }
    };

    template <typename F>
    inline mapped_t<F> mapped(F&& f)
    {
        return mapped_t<F>(std::forward<F>(f));
    }

    template <typename R, typename T, typename ...Params, typename ...Args>
    inline auto mapped(R(T::*mem_fun)(Params...) const, Args&&... args)
    {
        // be careful to referenced object lifetime
        return mapped([&, mem_fun](T const& t) { return (t.*mem_fun)(std::forward<Args>(args)...); });
    }

    template <typename F>
    class filtered_t
    {
        F f;
    public:
        explicit filtered_t(F f)
            : f(std::move(f))
        {
        }

        template <typename T, typename R = typename std::result_of<F(T)>::type>
        boost::optional<T> operator()(boost::optional<T> const& op) const
        {
            static_assert(std::is_convertible<R, bool>::value, "Pred must return bool type");
            if (!op) {
                return boost::none;
            }
            return f(op.get()) ? op : boost::none;
        }

        inline boost::none_t operator()(boost::none_t const&) const noexcept
        {
            return boost::none;
        }
    };

    template <typename F>
    inline filtered_t<F> filtered(F&& f)
    {
        return filtered_t<F>(std::forward<F>(f));
    }

    template <typename R, typename T, typename ...Params, typename ...Args>
    inline auto filtered(R(T::*mem_fun)(Params...) const, Args&&... args)
    {
        // be careful to referenced object lifetime
        return filtered([&, mem_fun](T const& t) { return (t.*mem_fun)(std::forward<Args>(args)...); });
    }

    struct to_optional_tag
    {
    };

    inline to_optional_tag to_optional() noexcept
    {
        return {};
    }

    template <typename Optional, typename Adaptor>
    inline typename std::enable_if<
        optional_utility_detail::is_optional<Optional>::value,
        optional_view<Adaptor, Optional>
    >::type operator|(Optional&& op, Adaptor&& adaptor)
    {
        return optional_view<Adaptor, Optional>(std::forward<Adaptor>(adaptor), std::forward<Optional>(op));
    }

    template <typename Optional>
    inline auto operator|(Optional&& op, to_optional_tag)
    {
        return op.evaluate();
    }

    template <typename T, typename Adaptor>
    inline optional_view<Adaptor, optional_wrapper<T>> operator|(boost::optional<T> op, Adaptor&& adaptor)
    {
        return optional_view<Adaptor, optional_wrapper<T>>(
            std::forward<Adaptor>(adaptor), optional_wrapper<T>{std::move(op)}
        );
    }

    template <typename T>
    inline boost::optional<T> operator|(boost::optional<T> op, to_optional_tag)
    {
        return std::move(op);
    }

    template <typename Adaptor>
    inline boost::none_t operator|(boost::none_t const&, Adaptor&&) noexcept
    {
        return boost::none;
    }
}

#endif
