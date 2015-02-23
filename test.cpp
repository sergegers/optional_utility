﻿#include "optional_utility.hpp"
#include <boost/optional/optional_io.hpp>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
using boost::optional;

// returns joined strings if both strings has a value. otherwise none;
optional<std::string> full_name(optional<std::string> const& first_name, optional<std::string> const& last_name)
{
    using optional_utility::flat_mapped;
    using optional_utility::mapped;
    return last_name | flat_mapped([&first_name](std::string const& ln) {
        return first_name | mapped([&ln](std::string const& fn) {
            return ln + " " + fn;
        });
    });
}

optional<int> divide(int dividend, int divisor)
{
    if (divisor == 0) {
        return boost::none;
    }
    return dividend / divisor;
}

int main()
{
    {
        // example of `mapped` and `flat_mapped`
        auto const first_name = boost::make_optional<std::string>("John");
        auto const last_name = boost::make_optional<std::string>("Smith");
        std::cout << full_name(first_name, last_name) << std::endl; // John Smith
        std::cout << full_name(first_name, boost::none) << std::endl; // --
        std::cout << full_name(boost::none, last_name) << std::endl; // --
        std::cout << full_name(boost::none, boost::none) << std::endl; // -- 
    }
    {

        // gets held value or default value
        using optional_utility::value_or;
        auto const op = divide(42, std::random_device{}() % 2);
        std::cout << value_or(op, 0) << std::endl; // 42 or 0
    }
    {
        using optional_utility::value_or_throw;
        optional<int> const op = boost::none;
        try {
            // you can throw whatever exception if optional is none
            value_or_throw<std::runtime_error>(op, "none!");
        } catch (std::exception const& e) {
            std::cout << e.what() << std::endl;
        }
    }
    // tests
    {
        using optional_utility::value;
        auto lval = boost::make_optional(0);
        auto const clval = boost::make_optional(1);
        auto rval = boost::make_optional(2);
        value(lval);
        value(clval);
        value(std::move(rval));
    }
    {
        using optional_utility::value_or;
        auto const clval = boost::make_optional(1);
        auto rval = boost::make_optional(2);
        value_or(clval, 42);
        value_or(std::move(rval), 42);
    }
    {
        using optional_utility::value_or_eval;
        auto lval = boost::make_optional(0);
        auto const clval = boost::make_optional(1);
        auto rval = boost::make_optional(2);
        value_or_eval(lval, [](){ return 42; });
        value_or_eval(clval, [](){ return 42; });
        value_or_eval(std::move(rval), [](){ return 42; });
    }
    {
        using optional_utility::value_or_throw;
        auto lval = boost::make_optional(0);
        auto const clval = boost::make_optional(1);
        auto rval = boost::make_optional(2);
        value_or_throw<std::runtime_error>(lval, "");
        value_or_throw<std::runtime_error>(clval, "");
        value_or_throw<std::runtime_error>(std::move(rval), "");
    }
}
