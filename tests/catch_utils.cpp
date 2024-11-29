// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
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
#include    <advgetopt/utils.h>

#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// snapdev
//
#include    <snapdev/safe_setenv.h>


// C++
//
#include    <fstream>
#include    <random>


// last include
//
#include    <snapdev/poison.h>



CATCH_TEST_CASE("utils_unquote", "[utils][valid]")
{
    CATCH_START_SECTION("utils_unquote: unquote, default pairs")
    {
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_unquote: unquote, brackets")
    {
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
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("utils_quote", "[utils][valid]")
{
    CATCH_START_SECTION("utils_quote: quote, default pairs")
    {
        CATCH_REQUIRE(advgetopt::quote("") == "\"\"");
        CATCH_REQUIRE(advgetopt::quote("a") == "\"a\"");
        CATCH_REQUIRE(advgetopt::quote("ab") == "\"ab\"");
        CATCH_REQUIRE(advgetopt::quote("abc") == "\"abc\"");

        CATCH_REQUIRE(advgetopt::quote("", '"') == "\"\"");
        CATCH_REQUIRE(advgetopt::quote("a", '"') == "\"a\"");
        CATCH_REQUIRE(advgetopt::quote("ab", '"') == "\"ab\"");
        CATCH_REQUIRE(advgetopt::quote("abc", '"') == "\"abc\"");
        CATCH_REQUIRE(advgetopt::quote("abcd", '"') == "\"abcd\"");

        CATCH_REQUIRE(advgetopt::quote("", '\'') == "''");
        CATCH_REQUIRE(advgetopt::quote("a", '\'') == "'a'");
        CATCH_REQUIRE(advgetopt::quote("ab", '\'') == "'ab'");
        CATCH_REQUIRE(advgetopt::quote("abc", '\'') == "'abc'");
        CATCH_REQUIRE(advgetopt::quote("abcd", '\'') == "'abcd'");

        CATCH_REQUIRE(advgetopt::quote("", '[', ']') == "[]");
        CATCH_REQUIRE(advgetopt::quote("a", '[', ']') == "[a]");
        CATCH_REQUIRE(advgetopt::quote("ab", '[', ']') == "[ab]");
        CATCH_REQUIRE(advgetopt::quote("abc", '[', ']') == "[abc]");
        CATCH_REQUIRE(advgetopt::quote("abcd", '[', ']') == "[abcd]");

        CATCH_REQUIRE(advgetopt::quote("[]", '[', ']') == "[\\[\\]]");
        CATCH_REQUIRE(advgetopt::quote("[a]", '[', ']') == "[\\[a\\]]");
        CATCH_REQUIRE(advgetopt::quote("[ab]", '[', ']') == "[\\[ab\\]]");
        CATCH_REQUIRE(advgetopt::quote("[abc]", '[', ']') == "[\\[abc\\]]");
        CATCH_REQUIRE(advgetopt::quote("[abcd]", '[', ']') == "[\\[abcd\\]]");
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("utils_split", "[utils][valid]")
{
    CATCH_START_SECTION("utils_split: split three words")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("test with spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with");
        CATCH_REQUIRE(result[2] == "spaces");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split three words, one with single quotes")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("test 'with quotes and' spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "spaces");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split three words, one with double quotes")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("test \"with quotes and\" spaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "spaces");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split three words, one with single quotes but no spaces")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("test'with quotes and'nospaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "nospaces");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split three words, one with double quotes but no spaces")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("test\"with quotes and\"nospaces"
                              , result
                              , {" "});
        CATCH_REQUIRE(result.size() == 3);
        CATCH_REQUIRE(result[0] == "test");
        CATCH_REQUIRE(result[1] == "with quotes and");
        CATCH_REQUIRE(result[2] == "nospaces");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, four separators")
    {
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, multiple/repeated separators")
    {
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, and empty entries")
    {
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, start/end with separator")
    {
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, unclosed double quote")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("\"unclosed quote|mark"
                              , result
                              , {"|"});
        CATCH_REQUIRE(result.size() == 1);
        CATCH_REQUIRE(result[0] == "unclosed quote|mark");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_split: split five words, unclosed single quote")
    {
        advgetopt::string_list_t result;
        advgetopt::split_string("here is an \"unclosed quote|mark"
                              , result
                              , {"|"," "});
        //CATCH_REQUIRE(result.size() == 4);
        CATCH_REQUIRE(result[0] == "here");
        CATCH_REQUIRE(result[1] == "is");
        CATCH_REQUIRE(result[2] == "an");
        CATCH_REQUIRE(result[3] == "unclosed quote|mark");
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("utils_insert_group_name", "[utils][valid]")
{
    CATCH_START_SECTION("utils_insert_group_name: Full insert")
    {
        // CONFIG FILE HAS NO EXTENSION
        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-path");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , "group-name"
                            , ""));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-path");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , "group-name"
                            , nullptr));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-path");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/project-name.d/50-path");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/project-name.d/50-path");
        }

        // CONFIG FILE HAS EXTENSION
        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , ""));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , nullptr));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/basename.ext"
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/project-name.d/50-basename.ext");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/basename.ext"
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "/this/is/a/project-name.d/50-basename.ext");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_insert_group_name: empty cases")
    {
        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , nullptr
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , nullptr
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , ""
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "/this/is/a/path"
                            , ""
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              ""
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              ""
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              ""
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              ""
                            , nullptr
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              ""
                            , nullptr
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_insert_group_name: cases")
    {
        CATCH_REQUIRE_THROWS_MATCHES(advgetopt::insert_group_name(
                          "/this-is-a-path"
                        , "group"
                        , "project")
                    , advgetopt::getopt_root_filename
                    , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: filename \"/this-is-a-path\" last slash (/) is at the start, which is not allowed."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_insert_group_name: basename only")
    {
        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "basename"
                            , nullptr
                            , "advgetopt"));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "advgetopt.d/50-basename");
        }

        {
            advgetopt::string_list_t fullname(advgetopt::insert_group_name(
                              "basename.ext"
                            , "advgetopt"
                            , nullptr));
            CATCH_REQUIRE(fullname.size() == 1);
            CATCH_REQUIRE(fullname[0] == "advgetopt.d/50-basename.ext");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_insert_group_name: actual list of files on disk")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("advgetopt-multi", "sorted-user-conf", false);

        // generate an array of numbers from 00 to 99
        //
        std::vector<int> numbers;
        for(int i = 0; i < 100; ++i)
        {
            numbers.push_back(i);
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(numbers.begin(), numbers.end(), g);
        int const max(rand() % 50 + 10);
        numbers.resize(max);
        std::string path(SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        std::string::size_type const pos(path.rfind('/'));
        path = path.substr(0, pos);
        advgetopt::string_list_t filenames;
        for(int i = 0; i < max; ++i)
        {
            std::stringstream ss;
            ss << path;
            ss << '/';
            int const n(numbers[i]);
            if(n < 10)
            {
                ss << '0';
            }
            ss << n;
            ss << "-sorted-user-conf.config";
            filenames.push_back(ss.str());
            std::ofstream conf;
            conf.open(ss.str(), std::ios_base::out);
            CATCH_REQUIRE(conf.is_open());
            conf << "# Config with a number" << std::endl;
            conf << "var=\"value: " << numbers[i] << "\"" << std::endl;
        }
        std::sort(filenames.begin(), filenames.end());
        std::string const last_filename(*filenames.rbegin());
        std::string::size_type const slash_pos(last_filename.rfind('/'));
        std::string const expected_var("value: " + last_filename.substr(slash_pos + 1, 2));

        advgetopt::string_list_t fullnames(advgetopt::insert_group_name(
                          SNAP_CATCH2_NAMESPACE::g_config_filename
                        , "advgetopt-multi"
                        , "multi-channels"));
        CATCH_REQUIRE(fullnames.size() == filenames.size());
        for(size_t idx(0); idx < filenames.size(); ++idx)
        {
            CATCH_REQUIRE(fullnames[idx] == filenames[idx]);
        }

        {
            std::ofstream conf;
            conf.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out);
            CATCH_REQUIRE(conf.is_open());
            conf << "# Original Config with a number" << std::endl;
            conf << "var=master value" << std::endl;

            // verify the master config file
            //
            advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename);
            advgetopt::conf_file::pointer_t config_file(advgetopt::conf_file::get_conf_file(setup));
            CATCH_REQUIRE(config_file->get_parameter("var") == "master value");
        }

        {
            // run a load to verify that we indeed get the last var=...
            // value and not some random entry
            //
            std::string temp_dir = SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config";
            char const * const dirs[] = {
                temp_dir.c_str(),
                nullptr
            };
            advgetopt::option opts[] = {
                advgetopt::define_option(
                      advgetopt::Name("var")
                    , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                    , advgetopt::Help("verify loading configuration files in a serie.")
                ),
                advgetopt::end_options()
            };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
            advgetopt::options_environment env = {
                .f_project_name = "sorted-configs",
                .f_group_name = "advgetopt-multi",
                .f_options = opts,
                .f_options_files_directory = nullptr,
                .f_environment_variable_name = nullptr,
                .f_environment_variable_intro = nullptr,
                .f_section_variables_name = nullptr,
                .f_configuration_files = nullptr,
                .f_configuration_filename = "sorted-user-conf.config",
                .f_configuration_directories = dirs,
                .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
                .f_help_header = nullptr,
                .f_help_footer = nullptr,
                .f_version = nullptr,
                .f_license = nullptr,
                .f_copyright = nullptr,
                .f_build_date = UTC_BUILD_DATE,
                .f_build_time = UTC_BUILD_TIME,
                .f_groups = nullptr
            };
#pragma GCC diagnostic pop
            char const * const argv[] = {
                "test",
                nullptr
            };
            advgetopt::getopt opt(env, 1, const_cast<char **>(argv));
            CATCH_REQUIRE(memcmp(&opt.get_options_environment(), &env, sizeof(env)) == 0);
            CATCH_REQUIRE(opt.get_string("var") == expected_var);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("utils_default_group_name", "[utils][valid]")
{
    CATCH_START_SECTION("utils_default_group_name: full insert")
    {
        // CONFIG FILE HAS NO EXTENSION
        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/config"
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-config");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/advgetopt"
                            , "group-name"
                            , ""));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-advgetopt");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/complete"
                            , "group-name"
                            , nullptr));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-complete");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/swapped"
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/50-swapped");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/null"
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/50-null");
        }

        // CONFIG FILE HAS EXTENSION
        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , ""));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , nullptr));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/50-basename.ext");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/50-basename.ext");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname == "/this/is/a/project-name.d/50-basename.ext");
        }

        // verify all valid priorities
        //
        for(int priority(0); priority < 10; ++priority)
        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , "project-name"
                            , priority));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/0"
                                        + std::to_string(priority)
                                        + "-basename.ext");
        }
        for(int priority(10); priority < 100; ++priority)
        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/basename.ext"
                            , "group-name"
                            , "project-name"
                            , priority));
            CATCH_REQUIRE(fullname == "/this/is/a/group-name.d/"
                                        + std::to_string(priority)
                                        + "-basename.ext");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_default_group_name: empty cases")
    {
        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/path"
                            , nullptr
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/path"
                            , nullptr
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/path"
                            , ""
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "/this/is/a/path"
                            , ""
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              ""
                            , "group-name"
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              ""
                            , ""
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              ""
                            , nullptr
                            , "project-name"));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              ""
                            , nullptr
                            , ""));
            CATCH_REQUIRE(fullname.empty());
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              ""
                            , nullptr
                            , nullptr));
            CATCH_REQUIRE(fullname.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_default_group_name: single '/' at the start")
    {
        CATCH_REQUIRE_THROWS_MATCHES(advgetopt::default_group_name(
                          "/this-is-a-path"
                        , "group"
                        , "project")
                    , advgetopt::getopt_root_filename
                    , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: filename \"/this-is-a-path\" starts with a slash (/), which is not allowed."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_default_group_name: invalid priority")
    {
        // verify that negative priorities are prevented
        //
        for(int priority(-20); priority < 0; ++priority)
        {
            CATCH_REQUIRE_THROWS_MATCHES(advgetopt::default_group_name(
                          "/this/is/a/basename.ext"
                        , ((rand() & 1) == 0 ? "group-name" : nullptr)
                        , "project-name"
                        , priority)
                    , advgetopt::getopt_invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: priority must be a number between 0 and 99 inclusive; "
                        + std::to_string(priority)
                        + " is invalid."));
        }

        // verify that large priorities are prevented
        //
        for(int priority(100); priority < 120; ++priority)
        {
            CATCH_REQUIRE_THROWS_MATCHES(advgetopt::default_group_name(
                          "/this/is/a/basename.ext"
                        , ((rand() & 1) == 0 ? "group-name" : nullptr)
                        , "project-name"
                        , priority)
                    , advgetopt::getopt_invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: priority must be a number between 0 and 99 inclusive; "
                        + std::to_string(priority)
                        + " is invalid."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_default_group_name: basename Only")
    {
        {
            std::string const fullname(advgetopt::default_group_name(
                              "basename"
                            , nullptr
                            , "advgetopt"));
            CATCH_REQUIRE(fullname == "advgetopt.d/50-basename");
        }

        {
            std::string const fullname(advgetopt::default_group_name(
                              "basename.ext"
                            , "advgetopt"
                            , nullptr));
            CATCH_REQUIRE(fullname == "advgetopt.d/50-basename.ext");
        }
    }
    CATCH_END_SECTION()
}







CATCH_TEST_CASE("utils_handle_user_directory", "[utils][valid]")
{
    CATCH_START_SECTION("utils_handle_user_directory: valid cases")
    {
        snapdev::safe_setenv env("HOME", "/home/advgetopt");

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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_handle_user_directory: $HOME is empty")
    {
        snapdev::safe_setenv env("HOME", "");

        {
            std::string result(advgetopt::handle_user_directory("~"));
            CATCH_REQUIRE(result == "~");
        }

        {
            std::string result(advgetopt::handle_user_directory("~/.config/advgetopt.conf"));
            CATCH_REQUIRE(result == "~/.config/advgetopt.conf");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_handle_user_directory: paths do not start with ~")
    {
        snapdev::safe_setenv env("HOME", "/home/advgetopt");

        {
            std::string result(advgetopt::handle_user_directory("/~"));
            CATCH_REQUIRE(result == "/~");
        }

        {
            std::string result(advgetopt::handle_user_directory("/~/.config/advgetopt.conf"));
            CATCH_REQUIRE(result == "/~/.config/advgetopt.conf");
        }
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("utils_true_false", "[utils][boolean]")
{
    CATCH_START_SECTION("utils_true_false: true values")
    {
        CATCH_REQUIRE(advgetopt::is_true("true"));
        CATCH_REQUIRE(advgetopt::is_true("on"));
        CATCH_REQUIRE(advgetopt::is_true("1"));

        CATCH_REQUIRE_FALSE(advgetopt::is_true("false"));
        CATCH_REQUIRE_FALSE(advgetopt::is_true("off"));
        CATCH_REQUIRE_FALSE(advgetopt::is_true("0"));

        CATCH_REQUIRE_FALSE(advgetopt::is_true("random"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_true_false: false values")
    {
        CATCH_REQUIRE(advgetopt::is_false("false"));
        CATCH_REQUIRE(advgetopt::is_false("off"));
        CATCH_REQUIRE(advgetopt::is_false("0"));

        CATCH_REQUIRE_FALSE(advgetopt::is_false("true"));
        CATCH_REQUIRE_FALSE(advgetopt::is_false("on"));
        CATCH_REQUIRE_FALSE(advgetopt::is_false("1"));

        CATCH_REQUIRE_FALSE(advgetopt::is_false("random"));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
