/*
 * Files:
 *    tests/validator.cpp
 *
 * License:
 *    Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 */

// self
//
#include "main.h"

// advgetopt lib
//
#include <advgetopt/exception.h>

// snapdev lib
//
#include <snapdev/safe_setenv.h>

// C++ lib
//
#include <fstream>


namespace
{

std::int64_t large_rnd()
{
    return (static_cast<std::int64_t>(rand()) <<  0)
         ^ (static_cast<std::int64_t>(rand()) << 16)
         ^ (static_cast<std::int64_t>(rand()) << 32)
         ^ (static_cast<std::int64_t>(rand()) << 48);
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








CATCH_TEST_CASE("invalid_validator", "[validator][invalid][validation]")
{
    CATCH_START_SECTION("Register duplicated factories")
        class duplicate_integer
            : public advgetopt::validator
        {
        public:
            virtual std::string const name() const
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
            virtual std::string get_name() const
            {
                return "integer";
            }

            virtual std::shared_ptr<advgetopt::validator> create(advgetopt::string_list_t const & data) const override
            {
                snap::NOTUSED(data); // ignore `data`
                return std::make_shared<duplicate_integer>();
            }
        };
        std::unique_ptr<advgetopt::validator_factory> factory(new duplicate_factory());
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::register_validator(*factory.get())
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "you have two or more validator factories named \"integer\"."));
    CATCH_END_SECTION()


    CATCH_START_SECTION("Verify invalid ranges")
        advgetopt::string_list_t range{
            "abc",
            "abc...6",
            "3...def",
            "10...1"};

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid value for your ranges; it must only digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: abc is not a valid value for your ranges; it must only digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: def is not a valid value for your ranges; it must only digits, optionally preceeded by a sign (+ or -) and not overflow an int64_t value.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: 10 has to be smaller or equal to 1; you have an invalid range.");

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", range));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify invalid regex flags")
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: unsupported regex flag f in regular expression \"/contact@.*\\..*/f\".");

        advgetopt::validator::pointer_t regex_validator(advgetopt::validator::create("regex", {"/contact@.*\\..*/f"}));

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify invalid regex flags")
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

        CATCH_REQUIRE(regex_validator != nullptr);
        CATCH_REQUIRE(regex_validator->name() == "regex");

        CATCH_REQUIRE_FALSE(regex_validator->validate("@m2osw."));
        CATCH_REQUIRE(regex_validator->validate("contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@m2osw.com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("Contact@M2OSW.com"));

        CATCH_REQUIRE_FALSE(regex_validator->validate("contact@m2osw:com"));
        CATCH_REQUIRE_FALSE(regex_validator->validate("contact!m2osw.com"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify regex refuses more than one parameter")
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 2 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex", {"[a-z]+", "[0-9]+"});

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 2 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex([a-z]+, [0-9]+)");

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 3 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex", {"[a-z]+", "[0-9]+", "[#!@]"});

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: validator_regex() only supports one parameter;"
                          " 3 were supplied;"
                          " single or double quotation may be required?");
        advgetopt::validator::create("regex(\"[a-z]+\", \"[0-9]+\", \"[#!@]\")");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify missing ')' in string based create")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::create("integer(1...7")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "invalid validator parameter definition: \"integer(1...7\", the ')' is missing."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::validator::create("regex([a-z]+")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "invalid validator parameter definition: \"regex([a-z]+\", the ')' is missing."));
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
