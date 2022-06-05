// Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/advgetopt
// contact@m2osw.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// advgetopt
//
#include    <advgetopt/validator_duration.h>
#include    <advgetopt/validator_size.h>

#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// snapdev
//
#include    <snapdev/ostream_int128.h>


// C++
//
#include    <cmath>
#include    <fstream>
#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>





namespace
{

struct duration_t
{
    char const * const      f_suffix = nullptr;
    double                  f_factor = 1.0;
};

constexpr duration_t const g_duration_suffixes[] =
{
    { "",                      1.0 },
    { "s",                     1.0 },
    { "second",                1.0 },
    { "seconds",               1.0 },

    { "m",                    -1.0 },   // may represent minutes or months
    { "minute",               60.0 },
    { "minutes",              60.0 },

    { "h",                  3600.0 },
    { "hour",               3600.0 },
    { "hours",              3600.0 },

    { "d",                 86400.0 },
    { "day",               86400.0 },
    { "days",              86400.0 },

    { "w",           86400.0 * 7.0 },
    { "week",        86400.0 * 7.0 },
    { "weeks",       86400.0 * 7.0 },

    { "month",      86400.0 * 30.0 },
    { "months",     86400.0 * 30.0 },

    { "y",         86400.0 * 365.0 },
    { "year",      86400.0 * 365.0 },
    { "years",     86400.0 * 365.0 },
};

struct size_suffix_t
{
    char const * const      f_suffix = nullptr;
    int                     f_base = 1000.0;
    int                     f_power = 0.0;
};

constexpr size_suffix_t const g_size_suffixes[] =
{
    { "",     1000, 0 },
    { "B",    1000, 0 },

    { "kB",   1000, 1 },
    { "KiB",  1024, 1 },

    { "MB",   1000, 2 },
    { "MiB",  1024, 2 },

    { "GB",   1000, 3 },
    { "GiB",  1024, 3 },

    { "TB",   1000, 4 },
    { "TiB",  1024, 4 },

    { "PB",   1000, 5 },
    { "PiB",  1024, 5 },

    { "EB",   1000, 6 },
    { "EiB",  1024, 6 },

    { "ZB",   1000, 7 },
    { "ZiB",  1024, 7 },

    { "YB",   1000, 8 },
    { "YiB",  1024, 8 },
};

std::int64_t large_rnd(bool zero_allowed = true)
{
    for(;;)
    {
        std::int64_t const result((static_cast<std::int64_t>(rand()) <<  0)
                                ^ (static_cast<std::int64_t>(rand()) << 16)
                                ^ (static_cast<std::int64_t>(rand()) << 32)
                                ^ (static_cast<std::int64_t>(rand()) << 48));
        if(result != 0
        || zero_allowed)
        {
            return result;
        }
    }
}

}



CATCH_TEST_CASE("unknown_validator", "[validator][valid][validation]")
{
    CATCH_START_SECTION("Undefined validator")
        // this is a valid case, it does not throw, it just returns a nullptr
        //
        CATCH_REQUIRE(advgetopt::validator::create("unknown", advgetopt::string_list_t()) == nullptr);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Empty string")
        CATCH_REQUIRE(advgetopt::validator::create(std::string()) == nullptr);
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("integer_validator", "[validator][valid][validation]")
{
    CATCH_START_SECTION("Verify the integer validator")
        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", advgetopt::string_list_t()));

        CATCH_REQUIRE(integer_validator != nullptr);
        CATCH_REQUIRE(integer_validator->name() == "integer");

        CATCH_REQUIRE_FALSE(integer_validator->validate(""));
        CATCH_REQUIRE_FALSE(integer_validator->validate("+"));
        CATCH_REQUIRE_FALSE(integer_validator->validate("-"));

        for(int idx(0); idx < 1000; ++idx)
        {
            std::int64_t value(large_rnd());
            std::string const v(std::to_string(value));

            CATCH_REQUIRE(integer_validator->validate(v));

            if(value >= 0)
            {
                CATCH_REQUIRE(integer_validator->validate('+' + v));
            }

            std::string const space_before(' ' + v);
            CATCH_REQUIRE_FALSE(integer_validator->validate(space_before));

            std::string const space_after(v + ' ');
            CATCH_REQUIRE_FALSE(integer_validator->validate(space_after));

            std::string const before(static_cast<char>(rand() % 26 + 'a') + v);
            CATCH_REQUIRE_FALSE(integer_validator->validate(before));

            std::string const after(v + static_cast<char>(rand() % 26 + 'a'));
            CATCH_REQUIRE_FALSE(integer_validator->validate(after));
        }

        // max number
        CATCH_REQUIRE(integer_validator->validate("9223372036854775807"));
        CATCH_REQUIRE(integer_validator->validate("+9223372036854775807"));

        // overflow
        CATCH_REQUIRE_FALSE(integer_validator->validate("9223372036854775808"));
        CATCH_REQUIRE_FALSE(integer_validator->validate("+9223372036854775808"));

        // min number
        CATCH_REQUIRE(integer_validator->validate("-9223372036854775808"));

        // underflow
        CATCH_REQUIRE_FALSE(integer_validator->validate("-9223372036854775809"));

        // too many digits
        CATCH_REQUIRE_FALSE(integer_validator->validate("92233720368547758091"));
        CATCH_REQUIRE_FALSE(integer_validator->validate("+92233720368547758092"));
        CATCH_REQUIRE_FALSE(integer_validator->validate("-92233720368547758093"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the integer ranges")
        bool had_standalone(false);
        for(int count(0); count < 20 || !had_standalone; ++count)
        {
            std::int64_t min(large_rnd());
            std::int64_t max(large_rnd());
            if(min > max)
            {
                std::swap(min, max);
            }

            std::string const & smin(std::to_string(min));
            std::string const & smax(std::to_string(max));

            std::string range("...");
            for(int three(0); three < 3; ++three)
            {
                if(rand() % 5 == 0)
                {
                    range = ' ' + range;
                }
                if(rand() % 5 == 0)
                {
                    range = range + ' ';
                }
            }
            range = smin + range + smax;
            for(int three(0); three < 3; ++three)
            {
                if(rand() % 5 == 0)
                {
                    range = ' ' + range;
                }
                if(rand() % 5 == 0)
                {
                    range = range + ' ';
                }
            }

            std::int64_t standalone(0);
            bool standalone_included(rand() % 4 == 0);
            if(standalone_included)
            {
                if(min == std::numeric_limits<std::int64_t>::min()
                && max == std::numeric_limits<std::int64_t>::max())
                {
                    standalone_included = false;
                }
                else
                {
                    had_standalone = true;
                    do
                    {
                        standalone = large_rnd();
                    }
                    while(standalone >= min && standalone <= max);

                    std::string sep(",");
                    if(rand() % 3 == 0)
                    {
                        sep = ' ' + sep;
                    }
                    if(rand() % 3 == 0)
                    {
                        sep = sep + ' ';
                    }
                    if(rand() % 2 == 0)
                    {
                        range = std::to_string(standalone) + "," + range;
                    }
                    else
                    {
                        range = range + "," + std::to_string(standalone);
                    }
                }
            }
            advgetopt::string_list_t range_list;
            advgetopt::split_string(range
                       , range_list
                       , {","});
            advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", range_list));

            CATCH_REQUIRE(integer_validator != nullptr);
            CATCH_REQUIRE(integer_validator->name() == "integer");

            for(int idx(0); idx < 1000; ++idx)
            {
                std::int64_t value(large_rnd());

                // force valid values otherwise we're likely to only have
                // invalid ones
                //
                if(idx % 10 == 0)
                {
                    value %= max - min + 1;
                    value += min;
                }
                else if(idx % 50 == 1 && standalone_included)
                {
                    value = standalone;
                }

                std::string const v(std::to_string(value));

                if((standalone_included && value == standalone)
                || (value >= min && value <= max))
                {
                    CATCH_REQUIRE(integer_validator->validate(v));
                }
                else
                {
                    CATCH_REQUIRE_FALSE(integer_validator->validate(v));
                }

                if(value >= 0)
                {
                    if((standalone_included && value == standalone)
                    || (value >= min && value <= max))
                    {
                        CATCH_REQUIRE(integer_validator->validate('+' + v));
                    }
                    else
                    {
                        CATCH_REQUIRE_FALSE(integer_validator->validate('+' + v));
                    }
                }

                std::string const space_before(' ' + v);
                CATCH_REQUIRE_FALSE(integer_validator->validate(space_before));

                std::string const space_after(v + ' ');
                CATCH_REQUIRE_FALSE(integer_validator->validate(space_after));

                std::string const before(static_cast<char>(rand() % 26 + 'a') + v);
                CATCH_REQUIRE_FALSE(integer_validator->validate(before));

                std::string const after(v + static_cast<char>(rand() % 26 + 'a'));
                CATCH_REQUIRE_FALSE(integer_validator->validate(after));
            }
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the integer standalone list")
        for(int count(0); count < 20; ++count)
        {
            int valid(rand() % 10 + 5);
            std::vector<std::int64_t> numbers;
            numbers.reserve(valid);
            std::string standalone_values;
            for(int idx(0); idx < valid; ++idx)
            {
                std::int64_t const value(large_rnd());
                numbers.push_back(value);
                std::string const & svalue(std::to_string(value));
                if(rand() % 5 == 0)
                {
                    standalone_values += ' ';
                }
                if(idx != 0)
                {
                    standalone_values += ',';
                }
                if(rand() % 5 == 0)
                {
                    standalone_values += ' ';
                }
                standalone_values += svalue;
            }
            if(rand() % 5 == 0)
            {
                standalone_values += ' ';
            }
            advgetopt::string_list_t range_list;
            advgetopt::split_string(standalone_values
                       , range_list
                       , {","});

            advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", range_list));

            CATCH_REQUIRE(integer_validator != nullptr);
            CATCH_REQUIRE(integer_validator->name() == "integer");

            for(size_t idx(0); idx < numbers.size(); ++idx)
            {
                std::string const svalue(std::to_string(numbers[idx]));

                CATCH_REQUIRE(integer_validator->validate(svalue));
            }

            for(int idx(0); idx < 1000; ++idx)
            {
                std::int64_t value;

                for(;;)
                {
                    value = large_rnd();
                    if(std::find(numbers.begin(), numbers.end(), value) == numbers.end())
                    {
                        break;
                    }
                }

                CATCH_REQUIRE_FALSE(integer_validator->validate(std::to_string(value)));
            }
        }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("double_validator", "[validator][valid][validation]")
{
    CATCH_START_SECTION("Verify the double validator")
        advgetopt::validator::pointer_t double_validator(advgetopt::validator::create("double", advgetopt::string_list_t()));

        CATCH_REQUIRE(double_validator != nullptr);
        CATCH_REQUIRE(double_validator->name() == "double");

        CATCH_REQUIRE_FALSE(double_validator->validate(""));
        CATCH_REQUIRE_FALSE(double_validator->validate("+"));
        CATCH_REQUIRE_FALSE(double_validator->validate("-"));
        CATCH_REQUIRE_FALSE(double_validator->validate("alpha"));

        for(int idx(0); idx < 1000; ++idx)
        {
            double value(static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false)));
            std::string const v(std::to_string(value));

            CATCH_REQUIRE(double_validator->validate(v));

            if(value >= 0)
            {
                CATCH_REQUIRE(double_validator->validate('+' + v));
            }

            std::string const space_before(' ' + v);
            CATCH_REQUIRE_FALSE(double_validator->validate(space_before));

            std::string const space_after(v + ' ');
            CATCH_REQUIRE_FALSE(double_validator->validate(space_after));

            std::string const before(static_cast<char>(rand() % 26 + 'a') + v);
            CATCH_REQUIRE_FALSE(double_validator->validate(before));

            std::string const after(v + static_cast<char>(rand() % 26 + 'a'));
            CATCH_REQUIRE_FALSE(double_validator->validate(after));
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the double ranges")
        bool had_standalone(false);
        for(int count(0); count < 20 || !had_standalone; ++count)
        {
            double min(static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false)));
            double max(static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false)));
            if(min > max)
            {
                std::swap(min, max);
            }

            std::string const & smin(std::to_string(min));
            std::string const & smax(std::to_string(max));

            std::string range("...");
            for(int three(0); three < 3; ++three)
            {
                if(rand() % 5 == 0)
                {
                    range = ' ' + range;
                }
                if(rand() % 5 == 0)
                {
                    range = range + ' ';
                }
            }
            range = smin + range + smax;
            for(int three(0); three < 3; ++three)
            {
                if(rand() % 5 == 0)
                {
                    range = ' ' + range;
                }
                if(rand() % 5 == 0)
                {
                    range = range + ' ';
                }
            }

            double standalone(0);
            bool standalone_included(rand() % 4 == 0);
            if(standalone_included)
            {
                if(min <= std::numeric_limits<double>::min()
                && max >= std::numeric_limits<double>::max())
                {
                    standalone_included = false;
                }
                else
                {
                    had_standalone = true;
                    do
                    {
                        standalone = static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false));
                    }
                    while(standalone >= min && standalone <= max);

                    std::string sep(",");
                    if(rand() % 3 == 0)
                    {
                        sep = ' ' + sep;
                    }
                    if(rand() % 3 == 0)
                    {
                        sep = sep + ' ';
                    }
                    if(rand() % 2 == 0)
                    {
                        range = std::to_string(standalone) + "," + range;
                    }
                    else
                    {
                        range = range + "," + std::to_string(standalone);
                    }
                }
            }
            advgetopt::string_list_t range_list;
            advgetopt::split_string(range
                       , range_list
                       , {","});
            advgetopt::validator::pointer_t double_validator(advgetopt::validator::create("double", range_list));

            CATCH_REQUIRE(double_validator != nullptr);
            CATCH_REQUIRE(double_validator->name() == "double");

            for(int idx(0); idx < 1000; ++idx)
            {
                double value(static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false)));

                // force valid values otherwise we're likely to only have
                // invalid ones
                //
                if(idx % 10 == 0)
                {
                    value = fmod(value, max - min + 1.0) + min;
                }
                else if(idx % 50 == 1 && standalone_included)
                {
                    value = standalone;
                }

                std::string const v(std::to_string(value));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
                if((standalone_included && value == standalone)
                || (value >= min && value <= max))
                {
                    CATCH_REQUIRE(double_validator->validate(v));
                }
                else
                {
                    CATCH_REQUIRE_FALSE(double_validator->validate(v));
                }

                if(value >= 0.0)
                {
                    if((standalone_included && value == standalone)
                    || (value >= min && value <= max))
                    {
                        CATCH_REQUIRE(double_validator->validate('+' + v));
                    }
                    else
                    {
                        CATCH_REQUIRE_FALSE(double_validator->validate('+' + v));
                    }
                }
#pragma GCC diagnostic pop

                std::string const space_before(' ' + v);
                CATCH_REQUIRE_FALSE(double_validator->validate(space_before));

                std::string const space_after(v + ' ');
                CATCH_REQUIRE_FALSE(double_validator->validate(space_after));

                std::string const before(static_cast<char>(rand() % 26 + 'a') + v);
                CATCH_REQUIRE_FALSE(double_validator->validate(before));

                std::string const after(v + static_cast<char>(rand() % 26 + 'a'));
                CATCH_REQUIRE_FALSE(double_validator->validate(after));
            }
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the double standalone list")
        for(int count(0); count < 20; ++count)
        {
            int valid(rand() % 10 + 5);
            std::vector<double> numbers;
            numbers.reserve(valid);
            std::string standalone_values;
            for(int idx(0); idx < valid; ++idx)
            {
                double const value(static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false)));
                numbers.push_back(value);
                std::string const & svalue(std::to_string(value));
                if(rand() % 5 == 0)
                {
                    standalone_values += ' ';
                }
                if(idx != 0)
                {
                    standalone_values += ',';
                }
                if(rand() % 5 == 0)
                {
                    standalone_values += ' ';
                }
                standalone_values += svalue;
            }
            if(rand() % 5 == 0)
            {
                standalone_values += ' ';
            }
            advgetopt::string_list_t range_list;
            advgetopt::split_string(standalone_values
                       , range_list
                       , {","});

            advgetopt::validator::pointer_t double_validator(advgetopt::validator::create("double", range_list));

            CATCH_REQUIRE(double_validator != nullptr);
            CATCH_REQUIRE(double_validator->name() == "double");

            for(size_t idx(0); idx < numbers.size(); ++idx)
            {
                std::string const svalue(std::to_string(numbers[idx]));

                CATCH_REQUIRE(double_validator->validate(svalue));
            }

            for(int idx(0); idx < 1000; ++idx)
            {
                std::int64_t value;

                for(;;)
                {
                    value = static_cast<double>(large_rnd()) / static_cast<double>(large_rnd(false));
                    if(std::find(numbers.begin(), numbers.end(), value) == numbers.end())
                    {
                        break;
                    }
                }

                CATCH_REQUIRE_FALSE(double_validator->validate(std::to_string(value)));
            }
        }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("duration_validator", "[validator][valid][validation]")
{
    CATCH_START_SECTION("Verify the duration validator (simple values)")
    {
        double duration(0.0);

        // simple seconds with decimal point
        //
        CATCH_REQUIRE(advgetopt::validator_duration::convert_string("22.3s", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 22.3, 0.0));

        // "seconds" is the default
        //
        CATCH_REQUIRE(advgetopt::validator_duration::convert_string("1.05", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 1.05, 0.0));

        // number can start with a decimal point
        //
        CATCH_REQUIRE(advgetopt::validator_duration::convert_string(".0503", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 0.0503, 0.0));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the duration validator (multiple values)")
    {
        double duration(0.0);
        CATCH_REQUIRE(advgetopt::validator_duration::convert_string("1d 3h 2m 15.3s", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 1.0 * 86400.0 + 3.0 * 3600.0 + 2.0 * 60.0 + 15.3, 0.0));

        // same in uppercase
        CATCH_REQUIRE(advgetopt::validator_duration::convert_string("1D 3H 2M 15.3S", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 1.0 * 86400.0 + 3.0 * 3600.0 + 2.0 * 60.0 + 15.3, 0.0));

        CATCH_REQUIRE(advgetopt::validator_duration::convert_string("3d 15h 52m 21.801s", 0, duration));
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(duration, 3.0 * 86400.0 + 15.0 * 3600.0 + 52.0 * 60.0 + 21.801, 0.0));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the duration validator (one value)")
    {
        // this test does not verify that double conversion works since we
        // have a separate test for that specific validator
        //
        for(int size(0); size < 3; ++size)
        {
            advgetopt::validator_duration::flag_t flg(advgetopt::validator_duration::VALIDATOR_DURATION_DEFAULT_FLAGS);
            advgetopt::string_list_t flags;
            if(size == 1)
            {
                flags.push_back("small");
            }
            else if(size == 2)
            {
                flags.push_back("large");
                flg = advgetopt::validator_duration::VALIDATOR_DURATION_LONG;
            }
            advgetopt::validator::pointer_t duration_validator(advgetopt::validator::create("duration", flags));

            CATCH_REQUIRE(duration_validator != nullptr);
            CATCH_REQUIRE(duration_validator->name() == "duration");

            for(int idx(0); idx < 1000; ++idx)
            {
                // use smaller values between 0 and 1
                // (the loop is to make sure we don't end up with "123e-10"
                // type of numbers... which do not work here)
                //
                double value(0.0);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
                do
                {
                    value = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
                }
                while(value < 0.0001 && value != 0.0);
#pragma GCC diagnostic pop
                if(rand() % 2 == 0)
                {
                    value *= -1.0;
                }
                std::stringstream ss;
                ss.precision(std::numeric_limits<double>::max_digits10);
                ss << value;
                std::string const v(ss.str());

                for(std::size_t i(0); i < std::size(g_duration_suffixes); ++i)
                {
                    for(int j(0); j <= 5; ++j)
                    {
                        std::string duration(v);
                        for(int k(0); k < j; ++k)
                        {
                            // any number of spaces in between are allowed
                            //
                            duration += ' ';
                        }
                        duration += g_duration_suffixes[i].f_suffix;

                        CATCH_REQUIRE(duration_validator->validate(duration));
                        if(value >= 0)
                        {
                            CATCH_REQUIRE(duration_validator->validate('+' + duration));
                        }

                        double result(0.0);
                        CATCH_REQUIRE(advgetopt::validator_duration::convert_string(duration, flg, result));
                        if(g_duration_suffixes[i].f_factor < 0.0)
                        {
                            // the 'm' special case
                            //
                            if(size == 2)
                            {
                                // 'large' -- 1 month
                                //
                                CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(result, value * (86400.0 * 30.0)));
                            }
                            else
                            {
                                // 'small' -- 1 minute
                                //
                                CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(result, value * 60.0));
                            }
                        }
                        else
                        {
                            CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(result, value * g_duration_suffixes[i].f_factor));
                        }
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("size_validator", "[validator][valid][validation]")
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    CATCH_START_SECTION("Verify the size validator")
    {
        // this test does not verify that double conversion works since we
        // have a separate test for that specific validator
        //
        for(int mode(0); mode < 3; ++mode)
        {
            advgetopt::validator_size::flag_t flg(advgetopt::validator_size::VALIDATOR_SIZE_DEFAULT_FLAGS);
            advgetopt::string_list_t flags;
            if(mode == 1)
            {
                flags.push_back("si");
            }
            else if(mode == 2)
            {
                flags.push_back("legacy");
                flg = advgetopt::validator_size::VALIDATOR_SIZE_POWER_OF_TWO;
            }
            advgetopt::validator::pointer_t size_validator(advgetopt::validator::create("size", flags));

            CATCH_REQUIRE(size_validator != nullptr);
            CATCH_REQUIRE(size_validator->name() == "size");

            for(int idx(0); idx < 1000; ++idx)
            {
                // use smaller values between 0 and about 5
                //
                double value(static_cast<double>(rand()) / static_cast<double>(RAND_MAX / 5));
                if(rand() % 2 == 0)
                {
                    value *= -1.0;
                }
                std::stringstream ss;
                ss.precision(std::numeric_limits<double>::max_digits10);
                ss << value;
                std::string const v(ss.str());

                for(std::size_t i(0); i < std::size(g_size_suffixes); ++i)
                {
                    for(int j(0); j <= 5; ++j)
                    {
                        std::string size(v);
                        for(int k(0); k < j; ++k)
                        {
                            // any number of spaces in between are allowed
                            //
                            size += ' ';
                        }
                        size += g_size_suffixes[i].f_suffix;

                        CATCH_REQUIRE(size_validator->validate(size));
                        if(value >= 0)
                        {
                            CATCH_REQUIRE(size_validator->validate('+' + size));
                        }

                        __int128 result(0.0);
                        CATCH_REQUIRE(advgetopt::validator_size::convert_string(size, flg, result));

                        long double const base(mode == 2 ? 1024.0L : g_size_suffixes[i].f_base);
                        long double expected(1);
                        for(int p(0); p < g_size_suffixes[i].f_power; ++p)
                        {
                            expected *= base;
                        }
                        __int128 int_expected(expected * static_cast<long double>(value));

//std::cerr << "converted [" << size << "] to [" << result << "] wanted [" << int_expected << "]\n";
                        CATCH_REQUIRE(result == int_expected);
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
#pragma GCC diagnostic pop
}




CATCH_TEST_CASE("regex_validator", "[validator][valid][validation]")
{
    CATCH_START_SECTION("Verify the regex validator")
        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {".*@.*\\..*"}));

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the regex string (case sensitive)")
        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {"/contact@.*\\..*/"}));

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify the regex string (case insensitive)")
        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {"/contact@.*\\..*/i"}));

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify direct regex string (case insensitive)")
        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("/contact@.*\\..*/i"));

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()
}








CATCH_TEST_CASE("invalid_validator_factory", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Register duplicated factories")
    {
        class duplicate_integer
            : public advgetopt::validator
        {
        public:
            virtual std::string name() const override
            {
                return "integer";
            }

            virtual bool validate(std::string const & value) const override
            {
                return value == "123";
            }
        };
        class duplicate_factory
            : public advgetopt::validator_factory
        {
        public:
            virtual std::string get_name() const override
            {
                return "integer";
            }

            virtual std::shared_ptr<advgetopt::validator> create(advgetopt::string_list_t const & data) const override
            {
                snapdev::NOT_USED(data); // ignore `data`
                return std::make_shared<duplicate_integer>();
            }
        };
        std::unique_ptr<advgetopt::validator_factory> factory(new duplicate_factory());
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::register_validator(*factory.get())
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: you have two or more validator factories named \"integer\"."));
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_validator_create", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Verify missing ')' in string based create")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::create("integer(1...7")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: invalid validator parameter definition: \"integer(1...7\", the ')' is missing."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::create("regex([a-z]+")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: invalid validator parameter definition: \"regex([a-z]+\", the ')' is missing."));
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_integer_validator", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Verify invalid integer ranges")
    {
        advgetopt::string_list_t range{
            "abc",
            "abc...6",
            "3...def",
            "10...1"};

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid standalone value for your ranges; it must only be digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid value for your range's start; it must only be digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: def is not a valid value for your range's end; it must only be digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: 10 has to be smaller or equal to 1; you have an invalid range.");

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", range));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_double_validator", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Verify invalid double ranges")
    {
        advgetopt::string_list_t range{
            "abc",
            "abc...6.3",
            "13.3...def",
            "10.5...1.2"};

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid standalone value; it must be a valid floating point, optionally preceeded by a sign (+ or -).");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid value for your range's start; it must be a valid floating point, optionally preceeded by a sign (+ or -).");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: def is not a valid value for your range's end; it must be a valid floating point, optionally preceeded by a sign (+ or -).");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: 10.5 has to be smaller or equal to 1.2; you have an invalid range.");

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("double", range));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_duration_validator", "[invalid][validation]")
{
    CATCH_START_SECTION("Verify invalid duration flags")
    {
        advgetopt::string_list_t range{
            "small",
            "medium",
            "large"};

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: medium is not a valid flag for the duration validator.");
        advgetopt::validator::pointer_t duration_validator(advgetopt::validator::create("duration", range));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE_FALSE(duration_validator->validate(""));
        CATCH_REQUIRE_FALSE(duration_validator->validate("  "));
        CATCH_REQUIRE_FALSE(duration_validator->validate("+"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("alpha"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("3.5 beta"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("7.5delta"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("+8.1 gamma"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-2.3eta"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-202.3   HERO"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-7.31Hr"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-1.32mom"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("-5.36 secs"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("28.901 wkS"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("28 YY"));
        CATCH_REQUIRE_FALSE(duration_validator->validate("2..8 year"));
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_size_validator", "[invalid][validation]")
{
    CATCH_START_SECTION("Verify invalid duration flags")
    {
        advgetopt::string_list_t flags{
            "si",
            "future",
            "legacy"};

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: future is not a valid flag for the size validator.");
        advgetopt::validator::pointer_t size_validator(advgetopt::validator::create("size", flags));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE_FALSE(size_validator->validate(""));
        CATCH_REQUIRE_FALSE(size_validator->validate("  "));
        CATCH_REQUIRE_FALSE(size_validator->validate("+"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-"));
        CATCH_REQUIRE_FALSE(size_validator->validate("size"));
        CATCH_REQUIRE_FALSE(size_validator->validate("3.5 large"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-1.31body"));
        CATCH_REQUIRE_FALSE(size_validator->validate("7.5small"));
        CATCH_REQUIRE_FALSE(size_validator->validate("+8.1 tiny"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-2.3medium"));
        CATCH_REQUIRE_FALSE(size_validator->validate("1000kbit"));
        CATCH_REQUIRE_FALSE(size_validator->validate("7 monster"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-101.101egret"));
        CATCH_REQUIRE_FALSE(size_validator->validate("11 products"));
        CATCH_REQUIRE_FALSE(size_validator->validate("1.01 tractor"));
        CATCH_REQUIRE_FALSE(size_validator->validate("+7.0 years"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-51.7zeroes"));
        CATCH_REQUIRE_FALSE(size_validator->validate("+121gruffalos"));
        CATCH_REQUIRE_FALSE(size_validator->validate("++1.7 KiB"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-+3.1 MiB"));
        CATCH_REQUIRE_FALSE(size_validator->validate("+-9.2 GiB"));
        CATCH_REQUIRE_FALSE(size_validator->validate("--19.4 PiB"));
        CATCH_REQUIRE_FALSE(size_validator->validate("-3.5.4B"));
    }
    CATCH_END_SECTION()
}

CATCH_TEST_CASE("invalid_regex_validator", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Verify invalid regex flags")
    {
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag f in regular expression \"/contact@.*\\..*/f\".");

        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {"/contact@.*\\..*/f"}));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify invalid regex: missing ending /")
    {
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag * in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag . in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag . in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag \\ in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag * in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag . in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag @ in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag t in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag c in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag a in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag t in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag n in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag o in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag c in regular expression \"/contact@.*\\..*\".");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid regex definition, ending / is missing in \"/contact@.*\\..*\".");

        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {"/contact@.*\\..*"}));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify regex refuses more than one parameter")
    {
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 2 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex", {"[a-z]+", "[0-9]+"});
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 2 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex([a-z]+, [0-9]+)");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 3 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex", {"[a-z]+", "[0-9]+", "[#!@]"});
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 3 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex(\"[a-z]+\", \"[0-9]+\", \"[#!@]\")");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
