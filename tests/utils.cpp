/*
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
#include <advgetopt/utils.h>

// snapdev lib
//
#include <snapdev/safe_setenv.h>



CATCH_TEST_CASE("utils_unquote", "[utils][valid]")
{
    CATCH_START_SECTION("Unquote, default pairs")
        CATCH_REQUIRE(advgetopt::unquote("") == "");
        CATCH_REQUIRE(advgetopt::unquote("a") == "a");
        CATCH_REQUIRE(advgetopt::unquote("ab") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("abc") == "abc");

        CATCH_REQUIRE(advgetopt::unquote("''") == "");
        CATCH_REQUIRE(advgetopt::unquote("'a'") == "a");
        CATCH_REQUIRE(advgetopt::unquote("'ab'") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("'abc'") == "abc");
        CATCH_REQUIRE(advgetopt::unquote("'abcd'") == "abcd");

        CATCH_REQUIRE(advgetopt::unquote("\"\"") == "");
        CATCH_REQUIRE(advgetopt::unquote("\"a\"") == "a");
        CATCH_REQUIRE(advgetopt::unquote("\"ab\"") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("\"abc\"") == "abc");
        CATCH_REQUIRE(advgetopt::unquote("\"abcd\"") == "abcd");

        CATCH_REQUIRE(advgetopt::unquote("\"'") == "\"'");
        CATCH_REQUIRE(advgetopt::unquote("\"a'") == "\"a'");
        CATCH_REQUIRE(advgetopt::unquote("\"ab'") == "\"ab'");
        CATCH_REQUIRE(advgetopt::unquote("\"abc'") == "\"abc'");
        CATCH_REQUIRE(advgetopt::unquote("\"abcd'") == "\"abcd'");

        CATCH_REQUIRE(advgetopt::unquote("'\"") == "'\"");
        CATCH_REQUIRE(advgetopt::unquote("'a\"") == "'a\"");
        CATCH_REQUIRE(advgetopt::unquote("'ab\"") == "'ab\"");
        CATCH_REQUIRE(advgetopt::unquote("'abc\"") == "'abc\"");
        CATCH_REQUIRE(advgetopt::unquote("'abcd\"") == "'abcd\"");

        CATCH_REQUIRE(advgetopt::unquote("\"") == "\"");
        CATCH_REQUIRE(advgetopt::unquote("\"a") == "\"a");
        CATCH_REQUIRE(advgetopt::unquote("\"ab") == "\"ab");
        CATCH_REQUIRE(advgetopt::unquote("\"abc") == "\"abc");
        CATCH_REQUIRE(advgetopt::unquote("\"abcd") == "\"abcd");

        CATCH_REQUIRE(advgetopt::unquote("'") == "'");
        CATCH_REQUIRE(advgetopt::unquote("'a") == "'a");
        CATCH_REQUIRE(advgetopt::unquote("'ab") == "'ab");
        CATCH_REQUIRE(advgetopt::unquote("'abc") == "'abc");
        CATCH_REQUIRE(advgetopt::unquote("'abcd") == "'abcd");

        CATCH_REQUIRE(advgetopt::unquote("'") == "'");
        CATCH_REQUIRE(advgetopt::unquote("a'") == "a'");
        CATCH_REQUIRE(advgetopt::unquote("ab'") == "ab'");
        CATCH_REQUIRE(advgetopt::unquote("abc'") == "abc'");
        CATCH_REQUIRE(advgetopt::unquote("abcd'") == "abcd'");

        CATCH_REQUIRE(advgetopt::unquote("\"") == "\"");
        CATCH_REQUIRE(advgetopt::unquote("a\"") == "a\"");
        CATCH_REQUIRE(advgetopt::unquote("ab\"") == "ab\"");
        CATCH_REQUIRE(advgetopt::unquote("abc\"") == "abc\"");
        CATCH_REQUIRE(advgetopt::unquote("abcd\"") == "abcd\"");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Unquote, brackets")
        CATCH_REQUIRE(advgetopt::unquote("", "[]<>{}") == "");
        CATCH_REQUIRE(advgetopt::unquote("a", "[]<>{}") == "a");
        CATCH_REQUIRE(advgetopt::unquote("ab", "[]<>{}") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("abc", "[]<>{}") == "abc");

        CATCH_REQUIRE(advgetopt::unquote("{}", "[]<>{}") == "");
        CATCH_REQUIRE(advgetopt::unquote("{a}", "[]<>{}") == "a");
        CATCH_REQUIRE(advgetopt::unquote("{ab}", "[]<>{}") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("{abc}", "[]<>{}") == "abc");
        CATCH_REQUIRE(advgetopt::unquote("{abcd}", "[]<>{}") == "abcd");

        CATCH_REQUIRE(advgetopt::unquote("[]", "[]<>{}") == "");
        CATCH_REQUIRE(advgetopt::unquote("[a]", "[]<>{}") == "a");
        CATCH_REQUIRE(advgetopt::unquote("[ab]", "[]<>{}") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("[abc]", "[]<>{}") == "abc");
        CATCH_REQUIRE(advgetopt::unquote("[abcd]", "[]<>{}") == "abcd");

        CATCH_REQUIRE(advgetopt::unquote("<>", "[]<>{}") == "");
        CATCH_REQUIRE(advgetopt::unquote("<a>", "[]<>{}") == "a");
        CATCH_REQUIRE(advgetopt::unquote("<ab>", "[]<>{}") == "ab");
        CATCH_REQUIRE(advgetopt::unquote("<abc>", "[]<>{}") == "abc");
        CATCH_REQUIRE(advgetopt::unquote("<abcd>", "[]<>{}") == "abcd");

        CATCH_REQUIRE(advgetopt::unquote("[}", "[]<>{}") == "[}");
        CATCH_REQUIRE(advgetopt::unquote("[a}", "[]<>{}") == "[a}");
        CATCH_REQUIRE(advgetopt::unquote("[ab}", "[]<>{}") == "[ab}");
        CATCH_REQUIRE(advgetopt::unquote("[abc}", "[]<>{}") == "[abc}");
        CATCH_REQUIRE(advgetopt::unquote("[abcd}", "[]<>{}") == "[abcd}");

        CATCH_REQUIRE(advgetopt::unquote("[>", "[]<>{}") == "[>");
        CATCH_REQUIRE(advgetopt::unquote("[a>", "[]<>{}") == "[a>");
        CATCH_REQUIRE(advgetopt::unquote("[ab>", "[]<>{}") == "[ab>");
        CATCH_REQUIRE(advgetopt::unquote("[abc>", "[]<>{}") == "[abc>");
        CATCH_REQUIRE(advgetopt::unquote("[abcd>", "[]<>{}") == "[abcd>");

        CATCH_REQUIRE(advgetopt::unquote("'\"", "[]<>{}") == "'\"");
        CATCH_REQUIRE(advgetopt::unquote("'a\"", "[]<>{}") == "'a\"");
        CATCH_REQUIRE(advgetopt::unquote("'ab\"", "[]<>{}") == "'ab\"");
        CATCH_REQUIRE(advgetopt::unquote("'abc\"", "[]<>{}") == "'abc\"");
        CATCH_REQUIRE(advgetopt::unquote("'abcd\"", "[]<>{}") == "'abcd\"");

        CATCH_REQUIRE(advgetopt::unquote("[", "[]<>{}") == "[");
        CATCH_REQUIRE(advgetopt::unquote("[a", "[]<>{}") == "[a");
        CATCH_REQUIRE(advgetopt::unquote("[ab", "[]<>{}") == "[ab");
        CATCH_REQUIRE(advgetopt::unquote("[abc", "[]<>{}") == "[abc");
        CATCH_REQUIRE(advgetopt::unquote("[abcd", "[]<>{}") == "[abcd");

        CATCH_REQUIRE(advgetopt::unquote("{", "[]<>{}") == "{");
        CATCH_REQUIRE(advgetopt::unquote("{a", "[]<>{}") == "{a");
        CATCH_REQUIRE(advgetopt::unquote("{ab", "[]<>{}") == "{ab");
        CATCH_REQUIRE(advgetopt::unquote("{abc", "[]<>{}") == "{abc");
        CATCH_REQUIRE(advgetopt::unquote("{abcd", "[]<>{}") == "{abcd");

        CATCH_REQUIRE(advgetopt::unquote("<", "[]<>{}") == "<");
        CATCH_REQUIRE(advgetopt::unquote("<a", "[]<>{}") == "<a");
        CATCH_REQUIRE(advgetopt::unquote("<ab", "[]<>{}") == "<ab");
        CATCH_REQUIRE(advgetopt::unquote("<abc", "[]<>{}") == "<abc");
        CATCH_REQUIRE(advgetopt::unquote("<abcd", "[]<>{}") == "<abcd");

        CATCH_REQUIRE(advgetopt::unquote("}", "[]<>{}") == "}");
        CATCH_REQUIRE(advgetopt::unquote("a}", "[]<>{}") == "a}");
        CATCH_REQUIRE(advgetopt::unquote("ab}", "[]<>{}") == "ab}");
        CATCH_REQUIRE(advgetopt::unquote("abc}", "[]<>{}") == "abc}");
        CATCH_REQUIRE(advgetopt::unquote("abcd}", "[]<>{}") == "abcd}");

        CATCH_REQUIRE(advgetopt::unquote("]", "[]<>{}") == "]");
        CATCH_REQUIRE(advgetopt::unquote("a]", "[]<>{}") == "a]");
        CATCH_REQUIRE(advgetopt::unquote("ab]", "[]<>{}") == "ab]");
        CATCH_REQUIRE(advgetopt::unquote("abc]", "[]<>{}") == "abc]");
        CATCH_REQUIRE(advgetopt::unquote("abcd]", "[]<>{}") == "abcd]");

        CATCH_REQUIRE(advgetopt::unquote(">", "[]<>{}") == ">");
        CATCH_REQUIRE(advgetopt::unquote("a>", "[]<>{}") == "a>");
        CATCH_REQUIRE(advgetopt::unquote("ab>", "[]<>{}") == "ab>");
        CATCH_REQUIRE(advgetopt::unquote("abc>", "[]<>{}") == "abc>");
        CATCH_REQUIRE(advgetopt::unquote("abcd>", "[]<>{}") == "abcd>");
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("utils_split", "[utils][valid]")
{
    CATCH_START_SECTION("Split three words")
        advgetopt::string_list_t result;
        advgetopt::split_string("test with spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with");
        CATCH_REQUIRE(result[2] == "spaces");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split three words, one with single quotes")
        advgetopt::string_list_t result;
        advgetopt::split_string("test 'with quotes and' spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "spaces");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split three words, one with double quotes")
        advgetopt::string_list_t result;
        advgetopt::split_string("test \"with quotes and\" spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "spaces");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split three words, one with single quotes but no spaces")
        advgetopt::string_list_t result;
        advgetopt::split_string("test'with quotes and'nospaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "nospaces");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split three words, one with double quotes but no spaces")
        advgetopt::string_list_t result;
        advgetopt::split_string("test\"with quotes and\"nospaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "nospaces");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, four separators")
        advgetopt::string_list_t result;
        advgetopt::split_string("test,with quite|many;separators"
                              , result
                              , {" ",",","|",";"});
        CATCH_REQUIRE(result.size() == 5);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with");
        CATCH_REQUIRE(result[2] == "quite");
        CATCH_REQUIRE(result[3] == "many");
        CATCH_REQUIRE(result[4] == "separators");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, multiple/repeated separators")
        advgetopt::string_list_t result;
        advgetopt::split_string("test, with quite|||many ; separators"
                              , result
                              , {" ",",","|",";"});
        CATCH_REQUIRE(result.size() == 5);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with");
        CATCH_REQUIRE(result[2] == "quite");
        CATCH_REQUIRE(result[3] == "many");
        CATCH_REQUIRE(result[4] == "separators");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, and empty entries")
        advgetopt::string_list_t result;
        advgetopt::split_string("|||test, with quite\"\"many ; ''separators''"
                              , result
                              , {" ",",","|",";"});
        CATCH_REQUIRE(result.size() == 5);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with");
        CATCH_REQUIRE(result[2] == "quite");
        CATCH_REQUIRE(result[3] == "many");
        CATCH_REQUIRE(result[4] == "separators");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, start/end with separator")
        advgetopt::string_list_t result;
        advgetopt::split_string("|start|and|end|with|separator|"
                              , result
                              , {"|"});
        CATCH_REQUIRE(result.size() == 5);
        CATCH_REQUIRE(result[0] == "start");
        CATCH_REQUIRE(result[1] == "and");
        CATCH_REQUIRE(result[2] == "end");
        CATCH_REQUIRE(result[3] == "with");
        CATCH_REQUIRE(result[4] == "separator");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, unclosed double quote")
        advgetopt::string_list_t result;
        advgetopt::split_string("\"unclosed quote|mark"
                              , result
                              , {"|"});
        CATCH_REQUIRE(result.size() == 1);
        CATCH_REQUIRE(result[0] == "unclosed quote|mark");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Split five words, unclosed single quote")
        advgetopt::string_list_t result;
        advgetopt::split_string("here is an \"unclosed quote|mark"
                              , result
                              , {"|"," "});
        //CATCH_REQUIRE(result.size() == 4);
        CATCH_REQUIRE(result[0] == "here");
        CATCH_REQUIRE(result[1] == "is");
        CATCH_REQUIRE(result[2] == "an");
        CATCH_REQUIRE(result[3] == "unclosed quote|mark");
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("utils_insert_project_name", "[utils][valid]")
{
    CATCH_START_SECTION("Full insert")
        {
            std::string fullname(advgetopt::insert_project_name(
                              "/this/is/a/path"
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/path");
        }

        {
            std::string fullname(advgetopt::insert_project_name(
                              "/this/is/a/basename.ext"
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/basename.ext");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Empty cases")
        {
            std::string fullname(advgetopt::insert_project_name(
                              "/this/is/a/path"
                            , nullptr));
            CATCH_REQUIRE(fullname == std::string());
        }

        {
            std::string fullname(advgetopt::insert_project_name(
                              "/this/is/a/path"
                            , ""));
            CATCH_REQUIRE(fullname == std::string());
        }

        {
            std::string fullname(advgetopt::insert_project_name(
                              ""
                            , "project-name"));
            CATCH_REQUIRE(fullname == std::string());
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Basename Only")
        {
            std::string fullname(advgetopt::insert_project_name(
                              "basename"
                            , "advgetopt"));
            CATCH_REQUIRE(fullname == "advgetopt.d/basename");
        }

        {
            std::string fullname(advgetopt::insert_project_name(
                              "basename.ext"
                            , "advgetopt"));
            CATCH_REQUIRE(fullname == "advgetopt.d/basename.ext");
        }
    CATCH_END_SECTION()
}







CATCH_TEST_CASE("utils_handle_user_directory", "[utils][valid]")
{
    CATCH_START_SECTION("Valid cases")
        snap::safe_setenv env("HOME", "/home/advgetopt");

        {
            std::string result(advgetopt::handle_user_directory("~"));
            CATCH_REQUIRE(result == "/home/advgetopt");
        }

        {
            std::string result(advgetopt::handle_user_directory("~/"));
            CATCH_REQUIRE(result == "/home/advgetopt/");
        }

        {
            std::string result(advgetopt::handle_user_directory("~/.config/advgetopt.conf"));
            CATCH_REQUIRE(result == "/home/advgetopt/.config/advgetopt.conf");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("$HOME is empty")
        snap::safe_setenv env("HOME", "");

        {
            std::string result(advgetopt::handle_user_directory("~"));
            CATCH_REQUIRE(result == "~");
        }

        {
            std::string result(advgetopt::handle_user_directory("~/.config/advgetopt.conf"));
            CATCH_REQUIRE(result == "~/.config/advgetopt.conf");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Paths do not start with ~")
        snap::safe_setenv env("HOME", "/home/advgetopt");

        {
            std::string result(advgetopt::handle_user_directory("/~"));
            CATCH_REQUIRE(result == "/~");
        }

        {
            std::string result(advgetopt::handle_user_directory("/~/.config/advgetopt.conf"));
            CATCH_REQUIRE(result == "/~/.config/advgetopt.conf");
        }
    CATCH_END_SECTION()
}







// vim: ts=4 sw=4 et
