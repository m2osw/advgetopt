// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// advgetopt
//
#include    <advgetopt/exception.h>


// C++
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>





CATCH_TEST_CASE("to_from_short_name", "[option_info][valid][basic][short_name]")
{
    CATCH_START_SECTION("to_from_short_name: short name to string and back")
    {
        // wc == U'\0' is a special case
        //
        CATCH_REQUIRE(advgetopt::NO_SHORT_NAME == U'\0');
        CATCH_REQUIRE(advgetopt::short_name_to_string(U'\0') == std::string());
        CATCH_REQUIRE(advgetopt::string_to_short_name(std::string()) == U'\0');

        for(char32_t wc(1); wc < 0x110000; ++wc)
        {
            if(wc == 0xD800)
            {
                wc = 0xE000;
            }
            std::string str(advgetopt::short_name_to_string(wc));
            CATCH_REQUIRE(advgetopt::string_to_short_name(str) == wc);

            // add a second character to prove that string_to_short_name()
            // only works with one character
            //
            char32_t const second_char(rand() % (0xD800 - 0x20) + 0x20);
            str += advgetopt::short_name_to_string(second_char);
            CATCH_REQUIRE(advgetopt::string_to_short_name(str) == advgetopt::NO_SHORT_NAME);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("parsing_string", "[environment][valid]")
{
    CATCH_START_SECTION("parsing_string: test parsing of empty environment strings")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("empty")
                , advgetopt::ShortName('e')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("the empty option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test parse_string()";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.is_defined("invalid-parameter")
                , advgetopt::getopt_initialization
                , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: function called too soon, parser is not done yet (i.e. is_defined(), get_string(), get_long(), get_double() cannot be called until the parser is done)"));

        opt.parse_string("");

        // a parse without anything means that we're still uninitialized
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.is_defined("invalid-parameter")
                , advgetopt::getopt_initialization
                , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: function called too soon, parser is not done yet (i.e. is_defined(), get_string(), get_long(), get_double() cannot be called until the parser is done)"));
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("option_info_basics", "[option_info][valid][basic]")
{
    CATCH_START_SECTION("option_info_basics: simple option (verify defaults)")
    {
        advgetopt::option_info verbose("verbose");

        CATCH_REQUIRE(verbose.get_name() == "verbose");
        CATCH_REQUIRE(verbose.get_short_name() == advgetopt::NO_SHORT_NAME);
        CATCH_REQUIRE_FALSE(verbose.is_default_option());
        CATCH_REQUIRE(verbose.get_flags() == 0);

        advgetopt::flag_t flag(1);
        do
        {
            CATCH_REQUIRE(verbose.has_flag(flag) == 0);
            flag <<= 1;
        }
        while(flag != 0);

        CATCH_REQUIRE_FALSE(verbose.has_default());
        CATCH_REQUIRE(verbose.get_default().empty());
        CATCH_REQUIRE(verbose.get_help().empty());

        CATCH_REQUIRE(verbose.get_alias_destination() == nullptr);
        CATCH_REQUIRE(verbose.get_multiple_separators().empty());
        CATCH_REQUIRE_FALSE(verbose.is_defined());
        CATCH_REQUIRE(verbose.size() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_basics: auto-default")
    {
        advgetopt::option_info auto_default("--");

        CATCH_REQUIRE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(auto_default.is_default_option());

        auto_default.add_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(auto_default.is_default_option());

        auto_default.remove_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE_FALSE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION)); // unfortunate?
        CATCH_REQUIRE(auto_default.is_default_option());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_basics: explicit default")
    {
        advgetopt::option_info explicit_default("filenames", 'f');

        CATCH_REQUIRE_FALSE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE_FALSE(explicit_default.is_default_option());

        explicit_default.add_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(explicit_default.is_default_option());

        explicit_default.remove_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE_FALSE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE_FALSE(explicit_default.is_default_option());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_flags", "[option_info][valid][flags]")
{
    CATCH_START_SECTION("option_info_flags: check flags")
    {
        advgetopt::option_info flags("flags", 'f');

        advgetopt::flag_t flag(1);
        do
        {
            CATCH_REQUIRE(flags.get_flags() == 0);

            advgetopt::flag_t clear(1);
            do
            {
                CATCH_REQUIRE_FALSE(flags.has_flag(clear));
                clear <<= 1;
            }
            while(clear != 0);

            flags.add_flag(flag);
            CATCH_REQUIRE(flags.has_flag(flag));
            CATCH_REQUIRE(flags.get_flags() == flag);

            advgetopt::flag_t set(1);
            do
            {
                if(set == flag)
                {
                    CATCH_REQUIRE(flags.has_flag(set));
                }
                else
                {
                    CATCH_REQUIRE_FALSE(flags.has_flag(set));
                }
                set <<= 1;
            }
            while(set != 0);

            flags.remove_flag(flag);
            CATCH_REQUIRE(flags.get_flags() == 0);

            clear = 1;
            do
            {
                CATCH_REQUIRE_FALSE(flags.has_flag(clear));
                clear <<= 1;
            }
            while(clear != 0);

            flag <<= 1;
        }
        while(flag != 0);

        flag = 1;
        do
        {
            CATCH_REQUIRE(flags.get_flags() == 0);

            advgetopt::flag_t clear(1);
            do
            {
                CATCH_REQUIRE_FALSE(flags.has_flag(clear));
                clear <<= 1;
            }
            while(clear != 0);

            flags.set_flags(flag);
            CATCH_REQUIRE(flags.has_flag(flag));
            CATCH_REQUIRE(flags.get_flags() == flag);

            advgetopt::flag_t set(1);
            do
            {
                if(set == flag)
                {
                    CATCH_REQUIRE(flags.has_flag(set));
                }
                else
                {
                    CATCH_REQUIRE_FALSE(flags.has_flag(set));
                }
                set <<= 1;
            }
            while(set != 0);

            flags.set_flags(0);
            CATCH_REQUIRE(flags.get_flags() == 0);

            clear = 1;
            do
            {
                CATCH_REQUIRE_FALSE(flags.has_flag(clear));
                clear <<= 1;
            }
            while(clear != 0);

            flag <<= 1;
        }
        while(flag != 0);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_default", "[option_info][valid][default]")
{
    CATCH_START_SECTION("option_info_default: set/remove default")
    {
        advgetopt::option_info with_default("with_default", 'w');

        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        with_default.set_default("working");
        CATCH_REQUIRE(with_default.has_default());
        CATCH_REQUIRE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default() == "working");

        with_default.remove_default();
        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        std::string const default_value("other");
        with_default.set_default(default_value);
        CATCH_REQUIRE(with_default.has_default());
        CATCH_REQUIRE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default() == default_value);

        with_default.remove_default();
        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        std::string const empty_default;
        with_default.set_default(empty_default);
        CATCH_REQUIRE(with_default.has_default());
        CATCH_REQUIRE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        with_default.remove_default();
        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        with_default.set_default(nullptr); // no effects
        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());

        std::string const null_value("null works too");
        with_default.set_default(null_value);
        CATCH_REQUIRE(with_default.has_default());
        CATCH_REQUIRE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default() == null_value);

        with_default.set_default(nullptr); // no effects
        CATCH_REQUIRE(with_default.has_default());
        CATCH_REQUIRE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default() == null_value);

        with_default.remove_default();
        CATCH_REQUIRE_FALSE(with_default.has_default());
        CATCH_REQUIRE_FALSE(with_default.has_flag(advgetopt::GETOPT_FLAG_HAS_DEFAULT));
        CATCH_REQUIRE(with_default.get_default().empty());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_help", "[option_info][valid][help]")
{
    CATCH_START_SECTION("option_info_help: check help")
    {
        advgetopt::option_info help("help", 'h');

        CATCH_REQUIRE(help.get_help().empty());

        help.set_help("help me");
        CATCH_REQUIRE(help.get_help() == "help me");

        std::string const msg("help msg");
        help.set_help(msg);
        CATCH_REQUIRE(help.get_help() == msg);

        help.set_help(nullptr);
        CATCH_REQUIRE(help.get_help() == msg);

        help.set_help("");
        CATCH_REQUIRE(help.get_help().empty());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_validator", "[option_info][valid][validator]")
{
    CATCH_START_SECTION("option_info_validator: check validator (one value)")
    {
        advgetopt::option_info auto_validate("validator", 'C');
        auto_validate.set_flags(advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "51");
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "51");
        CATCH_REQUIRE(auto_validate.get_long(0) == 51);
        CATCH_REQUIRE(auto_validate.source() == advgetopt::option_source_t::SOURCE_DIRECT);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", {"1","2","5","6","8"}));
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"51\" given to parameter --validator is not considered valid: out of range.");
        auto_validate.set_validator(integer_validator);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.get_validator() == integer_validator);

        auto_validate.set_value(0, "6", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_DYNAMIC);
        CATCH_REQUIRE(auto_validate.source() == advgetopt::option_source_t::SOURCE_DYNAMIC);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"3\" given to parameter --validator is not considered valid: out of range.");
        auto_validate.set_value(0, "3", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_CONFIGURATION);
        CATCH_REQUIRE(auto_validate.source() == advgetopt::option_source_t::SOURCE_UNDEFINED);    // it doesn't take... it gets cleared though
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"11\" given to parameter --validator is not considered valid: out of range.");
        auto_validate.set_value(0, "11", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_ENVIRONMENT_VARIABLE);
        CATCH_REQUIRE(auto_validate.source() == advgetopt::option_source_t::SOURCE_UNDEFINED);    // it doesn't take... it gets cleared though
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_validator: check validator (multiple values)")
    {
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "-15", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "-15");
        CATCH_REQUIRE(auto_validate.get_long(0) == -15);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", {"-1","2","5","6","18"}));
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"-15\" given to parameter --validator is not considered valid: out of range.");
        auto_validate.set_validator(integer_validator);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.get_validator() == integer_validator);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"3\" given to parameter --validator is not considered valid: out of range.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"11\" given to parameter --validator is not considered valid: out of range.");
        CATCH_REQUIRE_FALSE(auto_validate.set_multiple_values("6,3,18,11", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.size() == 2);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "18");
        CATCH_REQUIRE(auto_validate.get_long(1) == 18);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        CATCH_REQUIRE(auto_validate.set_multiple_values("6,3,18,11", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "3");
        CATCH_REQUIRE(auto_validate.get_long(1) == 3);
        CATCH_REQUIRE(auto_validate.get_value(2) == "18");
        CATCH_REQUIRE(auto_validate.get_long(2) == 18);
        CATCH_REQUIRE(auto_validate.get_value(3) == "11");
        CATCH_REQUIRE(auto_validate.get_long(3) == 11);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_validator: check integer validator from string (multiple values)")
    {
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "35", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "35");
        CATCH_REQUIRE(auto_validate.get_long(0) == 35);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"35\" given to parameter --validator is not considered valid: out of range.");
        auto_validate.set_validator("integer(-1,2,5,6,18,51966)");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.get_validator() != nullptr);
        CATCH_REQUIRE(auto_validate.get_validator()->name() == "integer");

        // test with the lowercase introducer
        //
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"0b11\" given to parameter --validator is not considered valid: out of range.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"0o13\" given to parameter --validator is not considered valid: out of range.");
        CATCH_REQUIRE_FALSE(auto_validate.set_multiple_values("0d6,0b11,0x12,0o13,0xcafe", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.size() == 3);
        CATCH_REQUIRE(auto_validate.get_value(0) == "0d6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "0x12");
        CATCH_REQUIRE(auto_validate.get_long(1) == 18);
        CATCH_REQUIRE(auto_validate.get_value(2) == "0xcafe");
        CATCH_REQUIRE(auto_validate.get_long(2) == 51966);

        // test with the uppercase introducer
        //
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"0B11\" given to parameter --validator is not considered valid: out of range.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"0O13\" given to parameter --validator is not considered valid: out of range.");
        CATCH_REQUIRE_FALSE(auto_validate.set_multiple_values("0D6,0B11,0X12,0O13,0XCAFE", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.size() == 3);
        CATCH_REQUIRE(auto_validate.get_value(0) == "0D6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "0X12");
        CATCH_REQUIRE(auto_validate.get_long(1) == 18);
        CATCH_REQUIRE(auto_validate.get_value(2) == "0XCAFE");
        CATCH_REQUIRE(auto_validate.get_long(2) == 51966);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        // here, however, we cannot use the introducer
        //
        CATCH_REQUIRE(auto_validate.set_multiple_values("6,3,18,11", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "3");
        CATCH_REQUIRE(auto_validate.get_long(1) == 3);
        CATCH_REQUIRE(auto_validate.get_value(2) == "18");
        CATCH_REQUIRE(auto_validate.get_long(2) == 18);
        CATCH_REQUIRE(auto_validate.get_value(3) == "11");
        CATCH_REQUIRE(auto_validate.get_long(3) == 11);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_validator: check regex validator from string (multiple values)")
    {
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "abc", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator("/^[a-z]+$/");
        CATCH_REQUIRE(auto_validate.get_validator() != nullptr);
        CATCH_REQUIRE(auto_validate.get_validator()->name() == "regex");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"33\" given to parameter --validator is not considered valid: did not match the regex.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"45\" given to parameter --validator is not considered valid: did not match the regex.");
        CATCH_REQUIRE_FALSE(auto_validate.set_multiple_values("abc,qqq,33,zac,pop,45", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");
        CATCH_REQUIRE(auto_validate.get_value(1) == "qqq");
        CATCH_REQUIRE(auto_validate.get_value(2) == "zac");
        CATCH_REQUIRE(auto_validate.get_value(3) == "pop");

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        CATCH_REQUIRE(auto_validate.set_multiple_values("abc,-56,zoc", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(auto_validate.size() == 3);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");
        CATCH_REQUIRE(auto_validate.get_value(1) == "-56");
        CATCH_REQUIRE(auto_validate.get_value(2) == "zoc");
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("option_info_alias", "[option_info][valid][alias]")
{
    CATCH_START_SECTION("option_info_alias: check alias")
    {
        advgetopt::option_info::pointer_t option(std::make_shared<advgetopt::option_info>("option", 'o'));
        advgetopt::option_info alias("alias", 'a');

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);

        alias.set_alias_destination(option);

        CATCH_REQUIRE(alias.get_alias_destination() == option);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_multiple_separators", "[option_info][valid][separators][multiple]")
{
    CATCH_START_SECTION("option_info_multiple_separators: check multiple separators")
    {
        advgetopt::option_info separators("names", 'n');

        separators.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_separators(nullptr);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        advgetopt::string_list_t empty_list;
        separators.set_multiple_separators(empty_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        char const * const empty_separator_list[]{
            nullptr
        };
        separators.set_multiple_separators(empty_separator_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        char const * const separator_list[]{
            ",",
            ";",
            " ",
            "^",
            nullptr
        };
        separators.set_multiple_separators(separator_list);
        CATCH_REQUIRE(separators.get_multiple_separators().size() == 4);

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 5);
        CATCH_REQUIRE(separators.get_value(0) == "n1");
        CATCH_REQUIRE(separators.get_value(1) == "n2");
        CATCH_REQUIRE(separators.get_value(2) == "n3");
        CATCH_REQUIRE(separators.get_value(3) == "n4");
        CATCH_REQUIRE(separators.get_value(4) == "n5");

        separators.set_multiple_separators(empty_separator_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        advgetopt::string_list_t list{
            ",",
            ";",
            " ",
            "^",
        };
        separators.set_multiple_separators(list);
        CATCH_REQUIRE(separators.get_multiple_separators().size() == 4);

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 5);
        CATCH_REQUIRE(separators.get_value(0) == "n1");
        CATCH_REQUIRE(separators.get_value(1) == "n2");
        CATCH_REQUIRE(separators.get_value(2) == "n3");
        CATCH_REQUIRE(separators.get_value(3) == "n4");
        CATCH_REQUIRE(separators.get_value(4) == "n5");

        separators.set_multiple_separators(empty_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        CATCH_REQUIRE(separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE));
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_add_value", "[option_info][valid][add][multiple]")
{
    CATCH_START_SECTION("option_info_add_value: add value, verify lock (add/remove flag explicitly)")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.add_value("value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        one_value.add_value("value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("value three", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("value four", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value four");

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_add_value: add value, verify integer")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.add_value("123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        one_value.add_value("456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("789", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("505", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "505");
        CATCH_REQUIRE(one_value.get_long(0) == 505);

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_add_value: add value, verify multiple strings")
    {
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.add_value("value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");

        multi_value.add_value("value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("value three", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("value four", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");
        CATCH_REQUIRE(multi_value.get_value(2) == "value four");

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_add_value: add value, verify multiple integers")
    {
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.add_value("123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        multi_value.add_value("456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("789", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("505", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);
        CATCH_REQUIRE(multi_value.get_value(2) == "505");
        CATCH_REQUIRE(multi_value.get_long(2) == 505);

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_set_value", "[option_info][valid][set][multiple]")
{
    CATCH_START_SECTION("option_info_set_value: set value, verify lock (use lock()/unlock() functions)")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        one_value.set_value(0, "value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.lock();

        one_value.set_value(0, "value three", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.unlock();

        one_value.set_value(0, "value four", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value four");

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_set_value: set value, verify integer")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        one_value.set_value(0, "456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.lock();

        one_value.set_value(0, "789", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.unlock();

        one_value.set_value(0, "505", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "505");
        CATCH_REQUIRE(one_value.get_long(0) == 505);

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_set_value: set value, verify multiple strings (with MULTIPLE & lock(false))")
    {
        advgetopt::option_info multi_value("names", 'n');

        multi_value.lock(false);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");

        multi_value.set_value(1, "value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.lock(false);

        multi_value.set_value(2, "value three", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.unlock();

        multi_value.set_value(2, "value four", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");
        CATCH_REQUIRE(multi_value.get_value(2) == "value four");

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_set_value: set value, verify multiple integers (with MULTIPLE & lock(false))")
    {
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.lock(false);

        multi_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        multi_value.set_value(1, "456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.lock(false);

        multi_value.set_value(2, "789", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.unlock();

        multi_value.set_value(2, "505", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);
        CATCH_REQUIRE(multi_value.get_value(2) == "505");
        CATCH_REQUIRE(multi_value.get_long(2) == 505);

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_section_functions", "[option_info][valid][add][section]")
{
    CATCH_START_SECTION("option_info_section_functions: value without sections")
    {
        advgetopt::option_info value("no-sections", 'z');

        CATCH_REQUIRE(value.get_basename() == "no-sections");
        CATCH_REQUIRE(value.get_section_name() == std::string());
        CATCH_REQUIRE(value.get_section_name_list().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_section_functions: value with one section")
    {
        advgetopt::option_info value("one::section", 'o');

        CATCH_REQUIRE(value.get_basename() == "section");
        CATCH_REQUIRE(value.get_section_name() == "one");
        advgetopt::string_list_t sections(value.get_section_name_list());
        CATCH_REQUIRE(sections.size() == 1);
        CATCH_REQUIRE(sections[0] == "one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_section_functions: value with two sections")
    {
        advgetopt::option_info value("one::two::section", 't');

        CATCH_REQUIRE(value.get_basename() == "section");
        CATCH_REQUIRE(value.get_section_name() == "one::two");
        advgetopt::string_list_t sections(value.get_section_name_list());
        CATCH_REQUIRE(sections.size() == 2);
        CATCH_REQUIRE(sections[0] == "one");
        CATCH_REQUIRE(sections[1] == "two");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("option_info_section_functions: value with three sections")
    {
        advgetopt::option_info value("s1::s2::s3::section", 'f');

        CATCH_REQUIRE(value.get_basename() == "section");
        CATCH_REQUIRE(value.get_section_name() == "s1::s2::s3");
        advgetopt::string_list_t sections(value.get_section_name_list());
        CATCH_REQUIRE(sections.size() == 3);
        CATCH_REQUIRE(sections[0] == "s1");
        CATCH_REQUIRE(sections[1] == "s2");
        CATCH_REQUIRE(sections[2] == "s3");
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("redefine_option_short_name", "[options][valid][config]")
{
    CATCH_START_SECTION("redefine_option_short_name: test adding '-<delta>' to '--config-dir'")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("group")
                , advgetopt::ShortName('g')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("group name.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "snaplog.conf";
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-g",
            "m2osw",
            "-L",
            "-\xE2\xB5\xA0",        // Delta character
            "/opt/advgetopt/config",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);
        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_option("config-dir") != nullptr);
        CATCH_REQUIRE(opt.get_option("config-dir") == opt.get_option("config_dir"));
        opt.set_short_name("config-dir", 0x2D60);

        opt.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == opt.get_option("invalid_parameter"));
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("group") != nullptr);
        CATCH_REQUIRE(opt.get_option('g') == opt.get_option("group"));
        CATCH_REQUIRE(opt.is_defined("group"));
        CATCH_REQUIRE(opt.get_string("group") == "m2osw");
        CATCH_REQUIRE(opt.get_string("group", 0) == "m2osw");
        CATCH_REQUIRE(opt.get_default("group").empty());
        CATCH_REQUIRE(opt.size("group") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') == opt.get_option("license"));
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the config-dir system parameter
        CATCH_REQUIRE(opt.get_option("config-dir") != nullptr);
        CATCH_REQUIRE(opt.get_option(static_cast<advgetopt::short_name_t>(0x2D60)) == opt.get_option("config-dir"));
        CATCH_REQUIRE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 1);
        CATCH_REQUIRE(opt.get_string("config-dir") == "/opt/advgetopt/config");

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()
}









CATCH_TEST_CASE("invalid_option_info", "[option_info][invalid]")
{
    CATCH_START_SECTION("invalid_option_info: no name")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): all options must at least have a long name."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("", 'v')
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): all options must at least have a long name (short name: 'v'.)"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info(std::string())
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): all options must at least have a long name."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info(std::string(), 'p')
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): all options must at least have a long name (short name: 'p'.)"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: default with short name")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("--", 'f')
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): the default parameter \"--\" cannot include a short name ('f'.)"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: long name cannot start with a dash (-)")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("--dashes")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): an option cannot start with a dash (-), \"--dashes\" is not valid."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: short name cannot be a dash (-)")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("dash", '-')
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::option_info(): the short name of an option cannot be the dash (-)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set value with undefined source")
    {
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.set_value(0, "true", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_UNDEFINED)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::set_value(): called with SOURCE_UNDEFINED (5)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set multiple values with undefined source")
    {
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.set_multiple_values("a,b,c", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_UNDEFINED)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                          "getopt_logic_error: option_info::set_multiple_values(): called with SOURCE_UNDEFINED (5)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: get value when undefined")
    {
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.get_value()
                , advgetopt::getopt_undefined
                , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: option_info::get_value(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: get long when undefined")
    {
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.get_long()
                , advgetopt::getopt_undefined
                , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: option_info::get_long(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: get long when undefined")
    {
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.get_double()
                , advgetopt::getopt_undefined
                , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: option_info::get_double(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: check alias of alias")
    {
        advgetopt::option_info::pointer_t option(std::make_shared<advgetopt::option_info>("option", 'o'));
        advgetopt::option_info alias("alias", 'a');

        option->add_flag(advgetopt::GETOPT_FLAG_ALIAS);

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);

        CATCH_REQUIRE_THROWS_MATCHES(
                  alias.set_alias_destination(option)
                , advgetopt::getopt_invalid
                , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: option_info::set_alias(): you can't set an alias as"
                          " an alias of another option."));

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set value, verify multiple strings (with MULTIPLE)")
    {
        advgetopt::option_info multi_value("names", 'n');

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.source() == advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        CATCH_REQUIRE_THROWS_MATCHES(
                  multi_value.set_value(2, "value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: option_info::set_value(): no value at index 2 and it is not the last available index + 1 (idx > 1) so you can't set this value (try add_value() maybe?)."));

        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set value, verify multiple integers (with MULTIPLE)")
    {
        advgetopt::option_info multi_value("names", 'n');

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        CATCH_REQUIRE_THROWS_MATCHES(
                  multi_value.set_value(2, "456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: option_info::set_value(): no value at index 2 and it is not the last available index + 1 (idx > 1) so you can't set this value (try add_value() maybe?)."));

        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set value, verify multiple strings (without MULTIPLE)")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "value one", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        CATCH_REQUIRE_THROWS_MATCHES(
                  one_value.set_value(1, "value two", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: option_info::set_value(): single value option \"--names\" does not accepts index 1 which is not 0."));

        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: set value, verify multiple integers (without MULTIPLE)")
    {
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "123", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        CATCH_REQUIRE_THROWS_MATCHES(
                  one_value.set_value(1, "456", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: option_info::set_value(): single value option \"--names\" does not accepts index 1 which is not 0."));

        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: long number too large")
    {
        advgetopt::option_info size("size", 's');

        CATCH_REQUIRE(size.size() == 0);

        // really too large
        //
        size.set_value(0, "100000000000000000000", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "100000000000000000000");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (100000000000000000000) in parameter --size at offset 0.");
        CATCH_REQUIRE(size.get_long(0) == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // too large by 1
        //
        size.set_value(0, "9223372036854775808", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "9223372036854775808");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (9223372036854775808) in parameter --size at offset 0.");
        CATCH_REQUIRE(size.get_long(0) == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // too small by 1
        //
        size.set_value(0, "-9223372036854775809", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "-9223372036854775809");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (-9223372036854775809) in parameter --size at offset 0.");
        CATCH_REQUIRE(size.get_long(0) == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // not a valid number
        //
        size.set_value(0, "97 potatoes", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "97 potatoes");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (97 potatoes) in parameter --size at offset 0.");
        CATCH_REQUIRE(size.get_long(0) == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: check multiple separators")
    {
        advgetopt::option_info separators("names", 'n');

        advgetopt::string_list_t list{
            ",",
            ";",
            " ",
            "^"
        };
        separators.set_multiple_separators(list);

        CATCH_REQUIRE_THROWS_MATCHES(
                  separators.set_multiple_values("n1,n2;n3 n4 ^ n5", advgetopt::string_list_t(), advgetopt::option_source_t::SOURCE_COMMAND_LINE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: option_info::set_multiple_value(): parameter --names expects zero or one parameter."
                    " The set_multiple_value() function should not be called with parameters that only accept one value."));

        CATCH_REQUIRE(separators.size() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_option_info: check invalid parameter (missing ')')")
    {
        advgetopt::option_info auto_validate("validator", 'C');
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: validator(): parameter list must end with ')'. Remaining input: \"...EOS\"");
        auto_validate.set_validator("regex('^[a-z]+$/'");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("check_invalid_config_dir_short_names", "[arguments][invalid][getopt][config]")
{
    CATCH_START_SECTION("check_invalid_config_dir_short_names: trying to set NO_SHORT_NAME as '--config-dir' short name (option_info)")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("print")
                , advgetopt::ShortName(U'p')
                , advgetopt::Flags(advgetopt::command_flags<>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        environment_options.f_configuration_filename = "snapwatchdog.conf";
        environment_options.f_help_header = "Usage: test --config-dir";

        advgetopt::getopt opt(environment_options);

        advgetopt::option_info::pointer_t o(opt.get_option("config-dir"));
        CATCH_REQUIRE(o != nullptr);
        CATCH_REQUIRE(o->get_short_name() == advgetopt::NO_SHORT_NAME);

        opt.set_short_name("config-dir", advgetopt::NO_SHORT_NAME);
        CATCH_REQUIRE(o->get_short_name() == advgetopt::NO_SHORT_NAME);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("check_invalid_config_dir_short_names: trying to change short name of '--version' (option_info)")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("print")
                , advgetopt::ShortName('p')
                , advgetopt::Flags(advgetopt::command_flags<>())
                , advgetopt::Help("print all info.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "";
        environment_options.f_help_header = "Usage: test --config-dir";

        advgetopt::getopt opt(environment_options);

        advgetopt::option_info::pointer_t o(opt.get_option("version"));
        CATCH_REQUIRE(o != nullptr);
        CATCH_REQUIRE(o->get_short_name() == U'V');
        CATCH_REQUIRE(o == opt.get_option(U'V'));
        CATCH_REQUIRE(opt.get_option(U'v') == nullptr);

        opt.set_short_name("version", U'V');   // keep uppercase...
        CATCH_REQUIRE(o->get_short_name() == U'V');
        CATCH_REQUIRE(o == opt.get_option(U'V'));
        CATCH_REQUIRE(opt.get_option(U'v') == nullptr);

        opt.set_short_name("version", U'v');   // set to lowercase...
        CATCH_REQUIRE(o->get_short_name() == U'v');
        CATCH_REQUIRE(o == opt.get_option(U'v'));
        CATCH_REQUIRE(opt.get_option(U'V') == nullptr);

        opt.set_short_name("version", advgetopt::NO_SHORT_NAME);   // remove completely...
        CATCH_REQUIRE(o->get_short_name() ==advgetopt::NO_SHORT_NAME);
        CATCH_REQUIRE(opt.get_option(U'V') == nullptr);
        CATCH_REQUIRE(opt.get_option(U'v') == nullptr);
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("check_option_callbacks", "[arguments][getopt][config][callback]")
{
    CATCH_START_SECTION("check_option_callbacks: check option callbacks")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("print")
                , advgetopt::ShortName(U'p')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "snapwatchdog.conf";
        environment_options.f_help_header = "Usage: test --config-dir";

        advgetopt::getopt opt(environment_options);

        struct print_expected
        {
        public:
            void set_value(std::string const & value)
            {
                f_value = value;
            }

            std::string const & value() const
            {
                return f_value;
            }

        private:
            std::string f_value = std::string();
        };

        struct print_callback
        {
        public:
            print_callback(print_expected * expected)
                : f_expected(expected)
            {
            }

            void operator() (advgetopt::option_info const & opt)
            {
                CATCH_REQUIRE(f_expected->value() == opt.get_value());
            }

        private:
            print_expected *    f_expected = nullptr;
        };

        print_expected expected1;
        print_callback cb1(&expected1);

        advgetopt::option_info::pointer_t print(opt.get_option("print"));
        CATCH_REQUIRE(print != nullptr);
        advgetopt::option_info::callback_id_t const id1(print->add_callback(cb1));

        expected1.set_value("color");
        print->set_value(0, "color");

        // test with a second callback, both are hit
        //
        print_expected expected2;
        print_callback cb2(&expected2);
        advgetopt::option_info::callback_id_t const id2(print->add_callback(cb2));

        expected1.set_value("black & white");
        expected2.set_value("black & white");
        print->set_value(0, "black & white");

        // call the remove without a valid identifier so a change still
        // applies
        //
        print->remove_callback(id2 + 10);

        expected1.set_value("stipple");
        expected2.set_value("stipple");
        print->set_value(0, "stipple");

        // remove the first callback and try again
        //
        print->remove_callback(id1);

        expected1.set_value("ignored");     // this was removed
        expected2.set_value("dithering");
        print->set_value(0, "dithering");

        // remove the second callback and try again
        //
        print->remove_callback(id2);

        expected2.set_value("not called anymore either");
        print->set_value(0, "cmyk");
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
