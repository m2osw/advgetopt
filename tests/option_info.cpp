/*
 * Files:
 *    tests/options_info.cpp
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

// C++ lib
//
#include <fstream>





CATCH_TEST_CASE("option_info_basics", "[option_info][valid][basic]")
{
    CATCH_START_SECTION("Simple option (verify defaults)")
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

        CATCH_REQUIRE(verbose.get_children().empty());
        CATCH_REQUIRE(verbose.get_child("non-existant") == nullptr);
        CATCH_REQUIRE(verbose.get_child('z') == nullptr);
        CATCH_REQUIRE(verbose.get_alias_destination() == nullptr);
        CATCH_REQUIRE(verbose.get_multiple_separators().empty());
        CATCH_REQUIRE_FALSE(verbose.is_defined());
        CATCH_REQUIRE(verbose.size() == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Auto-default")
        advgetopt::option_info auto_default("--");

        CATCH_REQUIRE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(auto_default.is_default_option());

        auto_default.add_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(auto_default.is_default_option());

        auto_default.remove_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE_FALSE(auto_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION)); // unfortunate?
        CATCH_REQUIRE(auto_default.is_default_option());
    CATCH_END_SECTION()

    CATCH_START_SECTION("Explicit default")
        advgetopt::option_info explicit_default("filenames", 'f');

        CATCH_REQUIRE_FALSE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE_FALSE(explicit_default.is_default_option());

        explicit_default.add_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE(explicit_default.is_default_option());

        explicit_default.remove_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION);

        CATCH_REQUIRE_FALSE(explicit_default.has_flag(advgetopt::GETOPT_FLAG_DEFAULT_OPTION));
        CATCH_REQUIRE_FALSE(explicit_default.is_default_option());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_flags", "[option_info][valid][flags]")
{
    CATCH_START_SECTION("Check flags")
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
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_default", "[option_info][valid][default]")
{
    CATCH_START_SECTION("Set/remove default")
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
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_help", "[option_info][valid][help]")
{
    CATCH_START_SECTION("Check help")
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
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_validator", "[option_info][valid][validator]")
{
    CATCH_START_SECTION("Check validator (one value)")
        advgetopt::option_info auto_validate("validator", 'C');

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "51");
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "51");
        CATCH_REQUIRE(auto_validate.get_long(0) == 51);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", {"1","2","5","6","8"}));
        auto_validate.set_validator(integer_validator);
        CATCH_REQUIRE(auto_validate.get_validator() == integer_validator);

        auto_validate.set_value(0, "6");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"3\" in parameter --validator is not considered valid.");
        auto_validate.set_value(0, "3");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"11\" in parameter --validator is not considered valid.");
        auto_validate.set_value(0, "11");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check validator (multiple values)")
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "-15");
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "-15");
        CATCH_REQUIRE(auto_validate.get_long(0) == -15);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        advgetopt::validator::pointer_t integer_validator(advgetopt::validator::create("integer", {"-1","2","5","6","18"}));
        auto_validate.set_validator(integer_validator);
        CATCH_REQUIRE(auto_validate.get_validator() == integer_validator);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"3\" (from \"6,3,18,11\") given to parameter --validator is not considered valid.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"11\" (from \"6,3,18,11\") given to parameter --validator is not considered valid.");
        auto_validate.set_multiple_value("6,3,18,11");
        CATCH_REQUIRE(auto_validate.size() == 2);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "18");
        CATCH_REQUIRE(auto_validate.get_long(1) == 18);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_multiple_value("6,3,18,11");
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "3");
        CATCH_REQUIRE(auto_validate.get_long(1) == 3);
        CATCH_REQUIRE(auto_validate.get_value(2) == "18");
        CATCH_REQUIRE(auto_validate.get_long(2) == 18);
        CATCH_REQUIRE(auto_validate.get_value(3) == "11");
        CATCH_REQUIRE(auto_validate.get_long(3) == 11);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check integer validator from string (multiple values)")
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "35");
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "35");
        CATCH_REQUIRE(auto_validate.get_long(0) == 35);

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator("integer(-1,2,5,6,18)");
        CATCH_REQUIRE(auto_validate.get_validator() != nullptr);
        CATCH_REQUIRE(auto_validate.get_validator()->name() == "integer");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"3\" (from \"6,3,18,11\") given to parameter --validator is not considered valid.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"11\" (from \"6,3,18,11\") given to parameter --validator is not considered valid.");
        auto_validate.set_multiple_value("6,3,18,11");
        CATCH_REQUIRE(auto_validate.size() == 2);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "18");
        CATCH_REQUIRE(auto_validate.get_long(1) == 18);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_multiple_value("6,3,18,11");
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "6");
        CATCH_REQUIRE(auto_validate.get_long(0) == 6);
        CATCH_REQUIRE(auto_validate.get_value(1) == "3");
        CATCH_REQUIRE(auto_validate.get_long(1) == 3);
        CATCH_REQUIRE(auto_validate.get_value(2) == "18");
        CATCH_REQUIRE(auto_validate.get_long(2) == 18);
        CATCH_REQUIRE(auto_validate.get_value(3) == "11");
        CATCH_REQUIRE(auto_validate.get_long(3) == 11);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check regex validator from string (multiple values)")
        advgetopt::option_info auto_validate("validator", 'C');

        auto_validate.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        advgetopt::string_list_t list{","};
        auto_validate.set_multiple_separators(list);

        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_value(0, "abc");
        CATCH_REQUIRE(auto_validate.size() == 1);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");

        auto_validate.set_validator(nullptr);
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_validator("/^[a-z]+$/");
        CATCH_REQUIRE(auto_validate.get_validator() != nullptr);
        CATCH_REQUIRE(auto_validate.get_validator()->name() == "regex");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"33\" (from \"abc,qqq,33,zac,pop,45\") given to parameter --validator is not considered valid.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"45\" (from \"abc,qqq,33,zac,pop,45\") given to parameter --validator is not considered valid.");
        auto_validate.set_multiple_value("abc,qqq,33,zac,pop,45");
        CATCH_REQUIRE(auto_validate.size() == 4);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");
        CATCH_REQUIRE(auto_validate.get_value(1) == "qqq");
        CATCH_REQUIRE(auto_validate.get_value(2) == "zac");
        CATCH_REQUIRE(auto_validate.get_value(3) == "pop");

        auto_validate.set_validator(std::string());
        CATCH_REQUIRE(auto_validate.get_validator() == nullptr);

        auto_validate.set_multiple_value("abc,-56,zoc");
        CATCH_REQUIRE(auto_validate.size() == 3);
        CATCH_REQUIRE(auto_validate.get_value(0) == "abc");
        CATCH_REQUIRE(auto_validate.get_value(1) == "-56");
        CATCH_REQUIRE(auto_validate.get_value(2) == "zoc");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_children", "[option_info][valid][child]")
{
    CATCH_START_SECTION("Check child without short bane")
        advgetopt::option_info root("root", 'r');
        advgetopt::option_info::pointer_t child(std::make_shared<advgetopt::option_info>("child"));

        root.add_child(nullptr);
        CATCH_REQUIRE(root.get_children().empty());
        CATCH_REQUIRE(root.get_child("child") == nullptr);
        CATCH_REQUIRE(root.get_child('c') == nullptr);

        root.add_child(advgetopt::option_info::pointer_t());
        CATCH_REQUIRE(root.get_children().empty());
        CATCH_REQUIRE(root.get_child("child") == nullptr);
        CATCH_REQUIRE(root.get_child('c') == nullptr);

        root.add_child(child);
        CATCH_REQUIRE(root.get_children().size() == 1);
        CATCH_REQUIRE(root.get_child("child") == child);
        CATCH_REQUIRE(root.get_child('c') == nullptr);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check child with short bane")
        advgetopt::option_info root("root", 'r');
        advgetopt::option_info::pointer_t child(std::make_shared<advgetopt::option_info>("child", 'c'));

        root.add_child(nullptr);
        CATCH_REQUIRE(root.get_children().empty());
        CATCH_REQUIRE(root.get_child("child") == nullptr);
        CATCH_REQUIRE(root.get_child('c') == nullptr);

        root.add_child(advgetopt::option_info::pointer_t());
        CATCH_REQUIRE(root.get_children().empty());
        CATCH_REQUIRE(root.get_child("child") == nullptr);
        CATCH_REQUIRE(root.get_child('c') == nullptr);

        root.add_child(child);
        CATCH_REQUIRE(root.get_children().size() == 1);
        CATCH_REQUIRE(root.get_child("child") == child);
        CATCH_REQUIRE(root.get_child('c') == child);
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_alias", "[option_info][valid][alias]")
{
    CATCH_START_SECTION("Check alias")
        advgetopt::option_info::pointer_t option(std::make_shared<advgetopt::option_info>("option", 'o'));
        advgetopt::option_info alias("alias", 'a');

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);

        alias.set_alias_destination(option);

        CATCH_REQUIRE(alias.get_alias_destination() == option);
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_multiple_separators", "[option_info][valid][separators][multiple]")
{
    CATCH_START_SECTION("Check multiple separators")
        advgetopt::option_info separators("names", 'n');

        separators.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_separators(nullptr);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        advgetopt::string_list_t empty_list;
        separators.set_multiple_separators(empty_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        char const * const empty_separator_list[]{
            nullptr
        };
        separators.set_multiple_separators(empty_separator_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
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

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 5);
        CATCH_REQUIRE(separators.get_value(0) == "n1");
        CATCH_REQUIRE(separators.get_value(1) == "n2");
        CATCH_REQUIRE(separators.get_value(2) == "n3");
        CATCH_REQUIRE(separators.get_value(3) == "n4");
        CATCH_REQUIRE(separators.get_value(4) == "n5");

        separators.set_multiple_separators(empty_separator_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");

        advgetopt::string_list_t list{
            ",",
            ";",
            " ",
            "^"
        };
        separators.set_multiple_separators(list);
        CATCH_REQUIRE(separators.get_multiple_separators().size() == 4);

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 5);
        CATCH_REQUIRE(separators.get_value(0) == "n1");
        CATCH_REQUIRE(separators.get_value(1) == "n2");
        CATCH_REQUIRE(separators.get_value(2) == "n3");
        CATCH_REQUIRE(separators.get_value(3) == "n4");
        CATCH_REQUIRE(separators.get_value(4) == "n5");

        separators.set_multiple_separators(empty_list);
        CATCH_REQUIRE(separators.get_multiple_separators().empty());

        separators.set_multiple_value("n1,n2;n3 n4 ^ n5");
        CATCH_REQUIRE(separators.size() == 1);
        CATCH_REQUIRE(separators.get_value(0) == "n1,n2;n3 n4 ^ n5");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_add_value", "[option_info][valid][add][multiple]")
{
    CATCH_START_SECTION("Add value, verify lock (add/remove flag explicitly)")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.add_value("value one");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        one_value.add_value("value two");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("value three");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("value four");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value four");

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "value one");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Add value, verify integer")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.add_value("123");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        one_value.add_value("456");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("789");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        one_value.add_value("505");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "505");
        CATCH_REQUIRE(one_value.get_long(0) == 505);

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "123");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Add value, verify multiple strings")
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.add_value("value one");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");

        multi_value.add_value("value two");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("value three");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("value four");
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");
        CATCH_REQUIRE(multi_value.get_value(2) == "value four");

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "value one");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Add value, verify multiple integers")
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.add_value("123");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        multi_value.add_value("456");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("789");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.remove_flag(advgetopt::GETOPT_FLAG_LOCK);

        multi_value.add_value("505");
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
        multi_value.set_value(0, "123");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_set_value", "[option_info][valid][set][multiple]")
{
    CATCH_START_SECTION("Set value, verify lock (use lock()/unlock() functions)")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "value one");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        one_value.set_value(0, "value two");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.lock();

        one_value.set_value(0, "value three");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value two");

        one_value.unlock();

        one_value.set_value(0, "value four");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value four");

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "value one");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify integer")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "123");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        one_value.set_value(0, "456");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.lock();

        one_value.set_value(0, "789");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "456");
        CATCH_REQUIRE(one_value.get_long(0) == 456);

        one_value.unlock();

        one_value.set_value(0, "505");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "505");
        CATCH_REQUIRE(one_value.get_long(0) == 505);

        one_value.reset();
        CATCH_REQUIRE(one_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        one_value.set_value(0, "123");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple strings (with MULTIPLE & lock(false))")
        advgetopt::option_info multi_value("names", 'n');

        multi_value.lock(false);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.set_value(0, "value one");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");

        multi_value.set_value(1, "value two");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.lock(false);

        multi_value.set_value(2, "value three");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");

        multi_value.unlock();

        multi_value.set_value(2, "value four");
        CATCH_REQUIRE(multi_value.size() == 3);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
        CATCH_REQUIRE(multi_value.get_value(1) == "value two");
        CATCH_REQUIRE(multi_value.get_value(2) == "value four");

        multi_value.reset();
        CATCH_REQUIRE(multi_value.size() == 0);

        // to verify that the f_integer was reset we kind of very much
        // have to re-add a value
        //
        multi_value.set_value(0, "value one");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple integers (with MULTIPLE & lock(false))")
        advgetopt::option_info multi_value("names", 'n');

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        multi_value.lock(false);

        multi_value.set_value(0, "123");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        multi_value.set_value(1, "456");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.lock(false);

        multi_value.set_value(2, "789");
        CATCH_REQUIRE(multi_value.size() == 2);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
        CATCH_REQUIRE(multi_value.get_value(1) == "456");
        CATCH_REQUIRE(multi_value.get_long(1) == 456);

        multi_value.unlock();

        multi_value.set_value(2, "505");
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
        multi_value.set_value(0, "123");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("invalid_option_info", "[option_info][invalid]")
{
    CATCH_START_SECTION("No name")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): all options must at least have a long name."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("", 'v')
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): all options must at least have a long name (short name: 'v'.)"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info(std::string())
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): all options must at least have a long name."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info(std::string(), 'p')
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): all options must at least have a long name (short name: 'p'.)"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Default with short name")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("--", 'f')
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): the default parameter \"--\" cannot include a short name ('f'.)"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Long name cannot start with a dash (-)")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("--dashes")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): an option cannot start with a dash (-), \"--dashes\" is not valid."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Short name cannot be a dash (-)")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::option_info("dash", '-')
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                          "option_info::option_info(): the short name of an option cannot be the dash (-)."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Validation without a value")
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.validates()
                , advgetopt::getopt_exception_undefined
                , Catch::Matchers::ExceptionMessage(
                          "option_info::get_value(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Get value when undefined")
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.get_value()
                , advgetopt::getopt_exception_undefined
                , Catch::Matchers::ExceptionMessage(
                          "option_info::get_value(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Get long when undefined")
        advgetopt::option_info verbose("verbose", 'v');
        CATCH_REQUIRE_THROWS_MATCHES(
                  verbose.get_long()
                , advgetopt::getopt_exception_undefined
                , Catch::Matchers::ExceptionMessage(
                          "option_info::get_long(): no value at index 0 (idx >= 0) for --verbose so you can't get this value."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check alias of alias")
        advgetopt::option_info::pointer_t option(std::make_shared<advgetopt::option_info>("option", 'o'));
        advgetopt::option_info alias("alias", 'a');

        option->add_flag(advgetopt::GETOPT_FLAG_ALIAS);

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);

        CATCH_REQUIRE_THROWS_MATCHES(
                  alias.set_alias_destination(option)
                , advgetopt::getopt_exception_invalid
                , Catch::Matchers::ExceptionMessage(
                          "option_info::set_alias(): you can't set an alias as"
                          " an alias of another option."));

        CATCH_REQUIRE(alias.get_alias_destination() == nullptr);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple strings (with MULTIPLE)")
        advgetopt::option_info multi_value("names", 'n');

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.set_value(0, "value one");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");

        CATCH_REQUIRE_THROWS_MATCHES(
                  multi_value.set_value(2, "value two")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "option_info::set_value(): no value at index 2 and it is not the last available index + 1 (idx > 1) so you can't set this value (try add_value() maybe?)."));

        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple integers (with MULTIPLE)")
        advgetopt::option_info multi_value("names", 'n');

        multi_value.add_flag(advgetopt::GETOPT_FLAG_MULTIPLE);

        CATCH_REQUIRE(multi_value.size() == 0);

        multi_value.set_value(0, "123");
        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);

        CATCH_REQUIRE_THROWS_MATCHES(
                  multi_value.set_value(2, "456")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "option_info::set_value(): no value at index 2 and it is not the last available index + 1 (idx > 1) so you can't set this value (try add_value() maybe?)."));

        CATCH_REQUIRE(multi_value.size() == 1);
        CATCH_REQUIRE(multi_value.get_value(0) == "123");
        CATCH_REQUIRE(multi_value.get_long(0) == 123);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple strings (without MULTIPLE)")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "value one");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");

        CATCH_REQUIRE_THROWS_MATCHES(
                  one_value.set_value(1, "value two")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "option_info::set_value(): single value option \"--names\" does not accepts index 1 which is not 0."));

        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "value one");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set value, verify multiple integers (without MULTIPLE)")
        advgetopt::option_info one_value("names", 'n');

        CATCH_REQUIRE(one_value.size() == 0);

        one_value.set_value(0, "123");
        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);

        CATCH_REQUIRE_THROWS_MATCHES(
                  one_value.set_value(1, "456")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "option_info::set_value(): single value option \"--names\" does not accepts index 1 which is not 0."));

        CATCH_REQUIRE(one_value.size() == 1);
        CATCH_REQUIRE(one_value.get_value(0) == "123");
        CATCH_REQUIRE(one_value.get_long(0) == 123);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Long number too large")
        advgetopt::option_info size("size", 's');

        CATCH_REQUIRE(size.size() == 0);

        // really too large
        //
        size.set_value(0, "100000000000000000000");
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "100000000000000000000");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (100000000000000000000) in parameter --size.");
        CATCH_REQUIRE(size.get_long(0) == -1);

        // too large by 1
        //
        size.set_value(0, "9223372036854775808");
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "9223372036854775808");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (9223372036854775808) in parameter --size.");
        CATCH_REQUIRE(size.get_long(0) == -1);

        // too small by 1
        //
        size.set_value(0, "-9223372036854775809");
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "-9223372036854775809");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (-9223372036854775809) in parameter --size.");
        CATCH_REQUIRE(size.get_long(0) == -1);

        // not a valid number
        //
        size.set_value(0, "97 potatoes");
        CATCH_REQUIRE(size.size() == 1);
        CATCH_REQUIRE(size.get_value(0) == "97 potatoes");

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (97 potatoes) in parameter --size.");
        CATCH_REQUIRE(size.get_long(0) == -1);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check multiple separators")
        advgetopt::option_info separators("names", 'n');

        advgetopt::string_list_t list{
            ",",
            ";",
            " ",
            "^"
        };
        separators.set_multiple_separators(list);

        CATCH_REQUIRE_THROWS_MATCHES(
                  separators.set_multiple_value("n1,n2;n3 n4 ^ n5")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "option_info::set_multiple_value(): parameter --names expects exactly one parameter."
                    " The set_multiple_value() function should not be called with parameters that only accept one value."));

        CATCH_REQUIRE(separators.size() == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check multiple separators")
        advgetopt::option_info auto_validate("validator", 'C');
        CATCH_REQUIRE_THROWS_MATCHES(
                  auto_validate.set_validator("regex('^[a-z]+$/'")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                    "invalid validator parameter definition: \"regex('^[a-z]+$/'\", the ')' is missing."));
    CATCH_END_SECTION()
}









// vim: ts=4 sw=4 et
