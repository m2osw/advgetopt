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

// self
//
#include    "catch_main.h"


// advgetopt lib
//
#include    <advgetopt/exception.h>


// libutf8 lib
//
#include    <libutf8/libutf8.h>


// C++ lib
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>





CATCH_TEST_CASE("option_info_ref", "[option_info][valid][reference]")
{
    CATCH_START_SECTION("Option info reference")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("test reference.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: verify references";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--reference",
            "1001",
            "--verbose",
            "loud",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // verify both parameters the "normal" way
        CATCH_REQUIRE(opt.get_option("reference") != nullptr);
        CATCH_REQUIRE(opt.size("reference") == 1);
        CATCH_REQUIRE(opt.get_string("reference") == "1001");

        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.size("verbose") == 1);
        CATCH_REQUIRE(opt.get_string("verbose") == "loud");

        // check the read-only verbose which does not create a reference
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["reference"] == "1001");
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["verbose"] == "loud");

        std::string const reference_value(const_cast<advgetopt::getopt const &>(opt)["reference"]);
        CATCH_REQUIRE(reference_value == "1001");
        std::string const verbose_value(const_cast<advgetopt::getopt const &>(opt)["verbose"]);
        CATCH_REQUIRE(verbose_value == "loud");

        // get a reference
        advgetopt::option_info_ref reference_ref(opt["reference"]);
        advgetopt::option_info_ref verbose_ref(opt["verbose"]);

        CATCH_REQUIRE_FALSE(reference_ref.empty());
        CATCH_REQUIRE_FALSE(verbose_ref.empty());

        CATCH_REQUIRE(reference_ref.length() == 4);
        CATCH_REQUIRE(reference_ref.size() == 4);
        CATCH_REQUIRE(verbose_ref.length() == 4);
        CATCH_REQUIRE(verbose_ref.size() == 4);

        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < reference_ref);
        CATCH_REQUIRE(reference_ref <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > reference_ref);
        CATCH_REQUIRE(reference_ref >= reference_ref);

        CATCH_REQUIRE_FALSE(reference_ref == verbose_ref);
        CATCH_REQUIRE(reference_ref != verbose_ref);
        CATCH_REQUIRE(reference_ref < verbose_ref);
        CATCH_REQUIRE(reference_ref <= verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref > verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref >= verbose_ref);

        reference_ref += "3";
        CATCH_REQUIRE(reference_ref == "10013");
        CATCH_REQUIRE("10013" == reference_ref);
        CATCH_REQUIRE(reference_ref != "17013");
        CATCH_REQUIRE("10413" != reference_ref);
        CATCH_REQUIRE(reference_ref < "20");
        CATCH_REQUIRE("1001" < reference_ref);
        CATCH_REQUIRE(reference_ref <= "10013");
        CATCH_REQUIRE("10013" <= reference_ref);
        CATCH_REQUIRE(reference_ref > "%");
        CATCH_REQUIRE("10014" > reference_ref);
        CATCH_REQUIRE(reference_ref >= "!");
        CATCH_REQUIRE("10013" >= reference_ref);

        std::string const new_value("zero");
        reference_ref = new_value;
        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE(reference_ref == new_value);
        CATCH_REQUIRE(new_value == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != new_value);
        CATCH_REQUIRE_FALSE(new_value != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < new_value);
        CATCH_REQUIRE_FALSE(new_value < reference_ref);
        CATCH_REQUIRE(reference_ref <= new_value);
        CATCH_REQUIRE(new_value <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > new_value);
        CATCH_REQUIRE_FALSE(new_value > reference_ref);
        CATCH_REQUIRE(reference_ref >= new_value);
        CATCH_REQUIRE(new_value >= reference_ref);

        reference_ref += verbose_ref;
        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE(reference_ref == "zeroloud");
        CATCH_REQUIRE("zeroloud" == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != "zeroloud");
        CATCH_REQUIRE_FALSE("zeroloud" != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < "zeroloud");
        CATCH_REQUIRE_FALSE("zeroloud" < reference_ref);
        CATCH_REQUIRE(reference_ref <= "zeroloud");
        CATCH_REQUIRE("zeroloud" <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > "zeroloud");
        CATCH_REQUIRE_FALSE("zeroloud" > reference_ref);
        CATCH_REQUIRE(reference_ref >= "zeroloud");
        CATCH_REQUIRE("zeroloud" >= reference_ref);

        reference_ref += '?';
        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE(reference_ref == "zeroloud?");
        CATCH_REQUIRE("zeroloud?" == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != "zeroloud?");
        CATCH_REQUIRE_FALSE("zeroloud?" != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < "zeroloud?");
        CATCH_REQUIRE_FALSE("zeroloud?" < reference_ref);
        CATCH_REQUIRE(reference_ref <= "zeroloud?");
        CATCH_REQUIRE("zeroloud?" <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > "zeroloud?");
        CATCH_REQUIRE_FALSE("zeroloud?" > reference_ref);
        CATCH_REQUIRE(reference_ref >= "zeroloud?");
        CATCH_REQUIRE("zeroloud?" >= reference_ref);

        CATCH_REQUIRE_FALSE(reference_ref + "more" == reference_ref);
        CATCH_REQUIRE(reference_ref + "more" == "zeroloud?more");
        CATCH_REQUIRE("zeroloud?more" == reference_ref + "more");
        CATCH_REQUIRE(reference_ref + std::string("extra") == "zeroloud?extra");
        CATCH_REQUIRE("zeroloud?extra" == reference_ref + std::string("extra"));
        CATCH_REQUIRE(reference_ref + verbose_ref == "zeroloud?loud");
        CATCH_REQUIRE("zeroloud?loud" == reference_ref + verbose_ref);

        CATCH_REQUIRE_FALSE(reference_ref + '+' == reference_ref);
        CATCH_REQUIRE(reference_ref + '+' == "zeroloud?+");
        CATCH_REQUIRE("zeroloud?+" == reference_ref + '+');
        CATCH_REQUIRE('+' + reference_ref == "+zeroloud?");
        CATCH_REQUIRE("+zeroloud?" == '+' + reference_ref);

        CATCH_REQUIRE(reference_ref + '\0' == reference_ref);
        CATCH_REQUIRE(reference_ref + '\0' == "zeroloud?");
        CATCH_REQUIRE("zeroloud?" == reference_ref + '\0');
        CATCH_REQUIRE('\0' + reference_ref == "zeroloud?");
        CATCH_REQUIRE("zeroloud?" == '\0' + reference_ref);

        char32_t c(rand() % 0xFFFFF + ' ');
        if(c >= 0xD800 && c < 0xE000)
        {
            c += 0x800;
        }

        CATCH_REQUIRE_FALSE(reference_ref + c == reference_ref);
        CATCH_REQUIRE(reference_ref + c == "zeroloud?" + libutf8::to_u8string(c));
        CATCH_REQUIRE("zeroloud?" + libutf8::to_u8string(c) == reference_ref + c);
        CATCH_REQUIRE(c + reference_ref == libutf8::to_u8string(c) + "zeroloud?");
        CATCH_REQUIRE(libutf8::to_u8string(c) + "zeroloud?" == c + reference_ref);

        c = U'\0';

        CATCH_REQUIRE(reference_ref + c == reference_ref);
        CATCH_REQUIRE(reference_ref + c == "zeroloud?");
        CATCH_REQUIRE("zeroloud?" == reference_ref + c);
        CATCH_REQUIRE(c + reference_ref == "zeroloud?");
        CATCH_REQUIRE("zeroloud?" == c + reference_ref);

        reference_ref = "reset";
        CATCH_REQUIRE('"' + reference_ref + '"' == "\"reset\"");
        CATCH_REQUIRE('\0' + reference_ref + '\0' == std::string("reset") + '\0');  // we do not control the second + here...
        CATCH_REQUIRE(c + reference_ref + c == "reset");

        reference_ref = verbose_ref;
        CATCH_REQUIRE('(' + reference_ref + ')' == "(loud)");
        CATCH_REQUIRE('\0' + reference_ref + '\0' == std::string("loud") + '\0');  // we do not control the second + here...
        CATCH_REQUIRE(c + reference_ref + c == "loud");

        std::string const secret("secret");
        reference_ref += ' ';
        reference_ref += secret;
        CATCH_REQUIRE('>' + reference_ref + '<' == ">loud secret<");
        char32_t const left(0x1D233);
        char32_t const right(0x1D234);
        CATCH_REQUIRE((left + (reference_ref + right)) == "\xF0\x9D\x88\xB3loud secret\xF0\x9D\x88\xB4");
        CATCH_REQUIRE(((left + reference_ref) + right) == "\xF0\x9D\x88\xB3loud secret\xF0\x9D\x88\xB4");
        CATCH_REQUIRE(c == U'\0');
        CATCH_REQUIRE((c + (reference_ref + c)) == "loud secret");
        CATCH_REQUIRE(((c + reference_ref) + c) == "loud secret");
        CATCH_REQUIRE(reference_ref + new_value == "loud secretzero");
        CATCH_REQUIRE(new_value + reference_ref == "zeroloud secret");
        CATCH_REQUIRE(reference_ref + " more" == "loud secret more");
        CATCH_REQUIRE("less " + reference_ref == "less loud secret");

        reference_ref = '#';
        CATCH_REQUIRE(reference_ref == "#");
        reference_ref += '\0';
        CATCH_REQUIRE(reference_ref == "#");
        reference_ref += c;
        CATCH_REQUIRE(reference_ref == "#");

        reference_ref = '\0';
        CATCH_REQUIRE(reference_ref == "");

        reference_ref = '?';
        CATCH_REQUIRE(reference_ref == "?");
        reference_ref += '\0';
        CATCH_REQUIRE(reference_ref == "?");
        reference_ref += c;
        CATCH_REQUIRE(reference_ref == "?");

        reference_ref = c;
        CATCH_REQUIRE(reference_ref == "");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Non-existant reference")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("test reference.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: verify references";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--reference",
            "1001",
            "--verbose",
            "loud",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // verify both parameters the "normal" way
        CATCH_REQUIRE(opt.get_option("reference") != nullptr);
        CATCH_REQUIRE(opt.size("reference") == 1);
        CATCH_REQUIRE(opt.get_string("reference") == "1001");

        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.size("verbose") == 1);
        CATCH_REQUIRE(opt.get_string("verbose") == "loud");

        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        // check the read-only verbose which does not create a reference
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["reference"] == "1001");
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["verbose"] == "loud");

        std::string const reference_value(const_cast<advgetopt::getopt const &>(opt)["reference"]);
        CATCH_REQUIRE(reference_value == "1001");
        std::string const verbose_value(const_cast<advgetopt::getopt const &>(opt)["verbose"]);
        CATCH_REQUIRE(verbose_value == "loud");

        // get references
        advgetopt::option_info_ref unknown_ref(opt["unknown"]);
        advgetopt::option_info_ref reference_ref(opt["reference"]);
        advgetopt::option_info_ref verbose_ref(opt["verbose"]);

        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        char const * null_string(nullptr);

        CATCH_REQUIRE(unknown_ref.empty());
        CATCH_REQUIRE(unknown_ref.length() == 0);
        CATCH_REQUIRE(unknown_ref.size() == 0);
        CATCH_REQUIRE(unknown_ref.get_long() == 0);
        CATCH_REQUIRE(static_cast<std::string>(unknown_ref) == "");
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(static_cast<bool>(unknown_ref));
        CATCH_REQUIRE(!unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref == nullptr);
        CATCH_REQUIRE(unknown_ref == "");
        CATCH_REQUIRE(unknown_ref == std::string());
        CATCH_REQUIRE_FALSE(unknown_ref == std::string("test"));
        CATCH_REQUIRE(nullptr == unknown_ref);
        CATCH_REQUIRE("" == unknown_ref);
        CATCH_REQUIRE(std::string() == unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") == unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref != nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref != "");
        CATCH_REQUIRE_FALSE(unknown_ref != std::string());
        CATCH_REQUIRE(unknown_ref != std::string("test"));
        CATCH_REQUIRE_FALSE(nullptr != unknown_ref);
        CATCH_REQUIRE_FALSE("" != unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() != unknown_ref);
        CATCH_REQUIRE(std::string("test") != unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref < nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref < "");
        CATCH_REQUIRE_FALSE(unknown_ref < std::string());
        CATCH_REQUIRE(unknown_ref < std::string("test"));
        CATCH_REQUIRE_FALSE(nullptr < unknown_ref);
        CATCH_REQUIRE_FALSE("" < unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() < unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") < unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref <= nullptr);
        CATCH_REQUIRE(unknown_ref <= "");
        CATCH_REQUIRE(unknown_ref <= std::string());
        CATCH_REQUIRE(unknown_ref <= std::string("test"));
        CATCH_REQUIRE(nullptr <= unknown_ref);
        CATCH_REQUIRE("" <= unknown_ref);
        CATCH_REQUIRE(std::string() <= unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") <= unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref > nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref > "");
        CATCH_REQUIRE_FALSE(unknown_ref > std::string());
        CATCH_REQUIRE_FALSE(unknown_ref > std::string("test"));
        CATCH_REQUIRE_FALSE(nullptr > unknown_ref);
        CATCH_REQUIRE_FALSE("" > unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() > unknown_ref);
        CATCH_REQUIRE(std::string("test") > unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref >= nullptr);
        CATCH_REQUIRE(unknown_ref >= "");
        CATCH_REQUIRE(unknown_ref >= std::string());
        CATCH_REQUIRE_FALSE(unknown_ref >= std::string("test"));
        CATCH_REQUIRE(nullptr >= unknown_ref);
        CATCH_REQUIRE("" >= unknown_ref);
        CATCH_REQUIRE(std::string() >= unknown_ref);
        CATCH_REQUIRE(std::string("test") >= unknown_ref);
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref + '\0' == "");
        CATCH_REQUIRE(unknown_ref + '<' == "<");
        CATCH_REQUIRE(unknown_ref + static_cast<char32_t>(U'\0') == "");
        CATCH_REQUIRE(unknown_ref + static_cast<char32_t>(U'\x2020') == "\xE2\x80\xA0");
        CATCH_REQUIRE(unknown_ref + null_string == "");
        CATCH_REQUIRE(unknown_ref + "abc\xE4\x81\x81" == "abc\xE4\x81\x81");
        CATCH_REQUIRE(unknown_ref + std::string("xyz\xE4\x9E\x99") == "xyz\xE4\x9E\x99");
        CATCH_REQUIRE(unknown_ref + reference_ref == "1001");
        CATCH_REQUIRE(unknown_ref + verbose_ref == "loud");
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        CATCH_REQUIRE('\0' + unknown_ref == "");
        CATCH_REQUIRE('<' + unknown_ref == "<");
        CATCH_REQUIRE(static_cast<char32_t>(U'\0') + unknown_ref == "");
        CATCH_REQUIRE(static_cast<char32_t>(U'\x2020') + unknown_ref == "\xE2\x80\xA0");
        CATCH_REQUIRE(null_string + unknown_ref == "");
        CATCH_REQUIRE("abc\xE4\x81\x81" + unknown_ref == "abc\xE4\x81\x81");
        CATCH_REQUIRE(std::string("xyz\xE4\x9E\x99") + unknown_ref == "xyz\xE4\x9E\x99");
        CATCH_REQUIRE(reference_ref + unknown_ref == "1001");
        CATCH_REQUIRE(verbose_ref + unknown_ref == "loud");
        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        unknown_ref = static_cast<char32_t>(U'\x4819'); // == '\xE4\xA0\x99'
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref.empty());
        CATCH_REQUIRE(unknown_ref.length() == 3);       // 3 UTF-8 bytes
        CATCH_REQUIRE(unknown_ref.size() == 3);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (\xE4\xA0\x99) in parameter --unknown at offset 0.");
        CATCH_REQUIRE(unknown_ref.get_long() == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(static_cast<std::string>(unknown_ref) == "\xE4\xA0\x99");
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE(static_cast<bool>(unknown_ref));
        CATCH_REQUIRE_FALSE(!unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref == nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref == "");
        CATCH_REQUIRE(unknown_ref == "\xE4\xA0\x99");
        CATCH_REQUIRE_FALSE(unknown_ref == std::string());
        CATCH_REQUIRE_FALSE(unknown_ref == std::string("test"));
        CATCH_REQUIRE(unknown_ref == std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE_FALSE(nullptr == unknown_ref);
        CATCH_REQUIRE_FALSE("" == unknown_ref);
        CATCH_REQUIRE("\xE4\xA0\x99" == unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() == unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") == unknown_ref);
        CATCH_REQUIRE(std::string("\xE4\xA0\x99") == unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref != nullptr);
        CATCH_REQUIRE(unknown_ref != "");
        CATCH_REQUIRE_FALSE(unknown_ref != "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref != std::string());
        CATCH_REQUIRE(unknown_ref != std::string("test"));
        CATCH_REQUIRE_FALSE(unknown_ref != std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE(nullptr != unknown_ref);
        CATCH_REQUIRE("" != unknown_ref);
        CATCH_REQUIRE_FALSE("\xE4\xA0\x99" != unknown_ref);
        CATCH_REQUIRE(std::string() != unknown_ref);
        CATCH_REQUIRE(std::string("test") != unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("\xE4\xA0\x99") != unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref < nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref < "");
        CATCH_REQUIRE_FALSE(unknown_ref < "\xE4\xA0\x99");
        CATCH_REQUIRE_FALSE(unknown_ref < std::string());
        CATCH_REQUIRE_FALSE(unknown_ref < std::string("test"));
        CATCH_REQUIRE_FALSE(unknown_ref < std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE(nullptr < unknown_ref);
        CATCH_REQUIRE("" < unknown_ref);
        CATCH_REQUIRE_FALSE("\xE4\xA0\x99" < unknown_ref);
        CATCH_REQUIRE(std::string() < unknown_ref);
        CATCH_REQUIRE(std::string("test") < unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("\xE4\xA0\x99") < unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE_FALSE(unknown_ref <= nullptr);
        CATCH_REQUIRE_FALSE(unknown_ref <= "");
        CATCH_REQUIRE(unknown_ref <= "\xE4\xA0\x99");
        CATCH_REQUIRE_FALSE(unknown_ref <= std::string());
        CATCH_REQUIRE_FALSE(unknown_ref <= std::string("test"));
        CATCH_REQUIRE(unknown_ref <= std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE(nullptr <= unknown_ref);
        CATCH_REQUIRE("" <= unknown_ref);
        CATCH_REQUIRE("\xE4\xA0\x99" <= unknown_ref);
        CATCH_REQUIRE(std::string() <= unknown_ref);
        CATCH_REQUIRE(std::string("test") <= unknown_ref);
        CATCH_REQUIRE(std::string("\xE4\xA0\x99") <= unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref > nullptr);
        CATCH_REQUIRE(unknown_ref > "");
        CATCH_REQUIRE_FALSE(unknown_ref > "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref > std::string());
        CATCH_REQUIRE(unknown_ref > std::string("test"));
        CATCH_REQUIRE_FALSE(unknown_ref > std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE_FALSE(nullptr > unknown_ref);
        CATCH_REQUIRE_FALSE("" > unknown_ref);
        CATCH_REQUIRE_FALSE("\xE4\xA0\x99" > unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() > unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") > unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("\xE4\xA0\x99") > unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref >= nullptr);
        CATCH_REQUIRE(unknown_ref >= "");
        CATCH_REQUIRE(unknown_ref >= "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref >= std::string());
        CATCH_REQUIRE(unknown_ref >= std::string("test"));
        CATCH_REQUIRE(unknown_ref >= std::string("\xE4\xA0\x99"));
        CATCH_REQUIRE_FALSE(nullptr >= unknown_ref);
        CATCH_REQUIRE_FALSE("" >= unknown_ref);
        CATCH_REQUIRE_FALSE(std::string() >= unknown_ref);
        CATCH_REQUIRE_FALSE(std::string("test") >= unknown_ref);
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE(unknown_ref + '\0' == "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref + '<' == "\xE4\xA0\x99<");
        CATCH_REQUIRE(unknown_ref + static_cast<char32_t>(U'\0') == "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref + static_cast<char32_t>(U'\x2020') == "\xE4\xA0\x99\xE2\x80\xA0");
        CATCH_REQUIRE(unknown_ref + null_string == "\xE4\xA0\x99");
        CATCH_REQUIRE(unknown_ref + "abc\xE4\x81\x81" == "\xE4\xA0\x99\x61\x62\x63\xE4\x81\x81");
        CATCH_REQUIRE(unknown_ref + std::string("xyz\xE4\x9E\x99") == "\xE4\xA0\x99xyz\xE4\x9E\x99");
        CATCH_REQUIRE(unknown_ref + reference_ref == "\xE4\xA0\x99\x31\x30\x30\x31");
        CATCH_REQUIRE(unknown_ref + verbose_ref == "\xE4\xA0\x99loud");
        CATCH_REQUIRE(opt.is_defined("unknown"));

        CATCH_REQUIRE('\0' + unknown_ref == "\xE4\xA0\x99");
        CATCH_REQUIRE('<' + unknown_ref == "<\xE4\xA0\x99");
        CATCH_REQUIRE(static_cast<char32_t>(U'\0') + unknown_ref == "\xE4\xA0\x99");
        CATCH_REQUIRE(static_cast<char32_t>(U'\x2020') + unknown_ref == "\xE2\x80\xA0\xE4\xA0\x99");
        CATCH_REQUIRE(null_string + unknown_ref == "\xE4\xA0\x99");
        CATCH_REQUIRE("abc\xE4\x81\x81" + unknown_ref == "abc\xE4\x81\x81\xE4\xA0\x99");
        CATCH_REQUIRE(std::string("xyz\xE4\x9E\x99") + unknown_ref == "xyz\xE4\x9E\x99\xE4\xA0\x99");
        CATCH_REQUIRE(reference_ref + unknown_ref == "1001\xE4\xA0\x99");
        CATCH_REQUIRE(verbose_ref + unknown_ref == "loud\xE4\xA0\x99");
        CATCH_REQUIRE(opt.is_defined("unknown"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Set non-existant reference + many CATCH_WHEN()")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("test reference.")
                , advgetopt::DefaultValue("978")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: verify references";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--reference",
            "3100",
            "--verbose",
            "silence",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // verify both parameters the "normal" way
        CATCH_REQUIRE(opt.get_option("reference") != nullptr);
        CATCH_REQUIRE(opt.size("reference") == 1);
        CATCH_REQUIRE(opt.get_string("reference") == "3100");
        CATCH_REQUIRE(opt.get_long("reference") == 3100);

        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.size("verbose") == 1);
        CATCH_REQUIRE(opt.get_string("verbose") == "silence");

        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

        // check the read-only verbose which does not create a reference
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["reference"] == "3100");
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["verbose"] == "silence");

        std::string const reference_value(const_cast<advgetopt::getopt const &>(opt)["reference"]);
        CATCH_REQUIRE(reference_value == "3100");
        std::string const verbose_value(const_cast<advgetopt::getopt const &>(opt)["verbose"]);
        CATCH_REQUIRE(verbose_value == "silence");

        // get references
        advgetopt::option_info_ref unknown_ref(opt["unknown"]);
        advgetopt::option_info_ref undefined_ref(opt["undefined"]);     // never set, used as rhs to test setting/adding with an undefined ref.
        advgetopt::option_info_ref reference_ref(opt["reference"]);
        advgetopt::option_info_ref verbose_ref(opt["verbose"]);

        CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));
        CATCH_REQUIRE(opt.is_defined("reference"));
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(reference_ref.get_long() == 3100);

        CATCH_WHEN("with = & zero char")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const c('\0');
            unknown_ref = c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & valid char")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const c(rand() % 26 + 'a');
            unknown_ref = c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            s += c;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            char const d(rand() % 26 + 'a');
            unknown_ref += d;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            s += d;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & zero char32_t")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char32_t const c(U'\0');
            unknown_ref = c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & valid char32_t")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char32_t c((rand() & 0xFFFFF) + ' ');
            if(c >= 0xD800 && c < 0xE000)
            {
                c += 0x0800;
            }
            unknown_ref = c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            s += libutf8::to_u8string(c);
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            char32_t d((rand() & 0xFFFFF) + ' ');
            if(d >= 0xD800 && d < 0xE000)
            {
                d += 0x0800;
            }
            unknown_ref += d;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            s += libutf8::to_u8string(d);
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & nullptr of 'char const *'")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const * str(nullptr);
            unknown_ref = str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & empty 'char const *' string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const * str("");
            unknown_ref = str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & valid `char const *`")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            size_t max(rand() % 10 + 1);
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                str += libutf8::to_u8string(c);
            }
            unknown_ref = str.c_str();

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == str);

            CATCH_REQUIRE(unknown_ref == str);

            std::string add;
            max = rand() % 10 + 1;
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                add += libutf8::to_u8string(c);
            }
            unknown_ref += add.c_str();

            CATCH_REQUIRE(opt.is_defined("unknown"));

            str += add;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == str);

            CATCH_REQUIRE(unknown_ref == str);
        }

        CATCH_WHEN("with = & empty std::string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            unknown_ref = str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);

            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with = & valid std::string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            size_t max(rand() % 10 + 1);
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                str += libutf8::to_u8string(c);
            }
            unknown_ref = str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == str);

            CATCH_REQUIRE(unknown_ref == str);

            std::string add;
            max = rand() % 10 + 1;
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                add += libutf8::to_u8string(c);
            }
            unknown_ref += add;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            str += add;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == str);

            CATCH_REQUIRE(unknown_ref == str);
        }

        CATCH_WHEN("with = & unknown reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref = undefined_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == "");

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == std::string());

            CATCH_REQUIRE(unknown_ref == std::string());
        }

        CATCH_WHEN("with = & self reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref = unknown_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == "");

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == std::string());

            CATCH_REQUIRE(unknown_ref == std::string());

            CATCH_REQUIRE(unknown_ref == unknown_ref);
        }

        CATCH_WHEN("with = & known reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref = verbose_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == verbose_value);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == verbose_value);

            CATCH_REQUIRE(unknown_ref == verbose_value);

            unknown_ref += reference_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == verbose_value + reference_value);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == verbose_value + reference_value);

            CATCH_REQUIRE(unknown_ref == verbose_value + reference_value);
        }

        CATCH_WHEN("with += & zero char")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const c('\0');
            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & valid char")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const c(rand() % 26 + 'a');
            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            s += c;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & zero char32_t")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char32_t const c(U'\0');
            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & valid char32_t")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char32_t c((rand() & 0xFFFFF) + ' ');
            if(c >= 0xD800 && c < 0xE000)
            {
                c += 0x0800;
            }

            unknown_ref += c;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            s += libutf8::to_u8string(c);
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & nullptr of 'char const *'")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const * str(nullptr);

            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & empty 'char const *' string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            char const * str("");

            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & valid `char const *`")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            size_t max(rand() % 10 + 1);
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                str += libutf8::to_u8string(c);
            }

            unknown_ref += str.c_str();

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == str);

            CATCH_REQUIRE(unknown_ref == str);
        }

        CATCH_WHEN("with += & empty std::string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            std::string s;
            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == s);

            std::string const unknown_value2(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value2 == s);

            CATCH_REQUIRE(unknown_ref == s);
        }

        CATCH_WHEN("with += & valid std::string")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            std::string str;
            size_t max(rand() % 10 + 1);
            for(size_t idx(0); idx < max; ++idx)
            {
                char32_t c((rand() & 0xFFFFF) + ' ');
                if(c >= 0xD800 && c < 0xE000)
                {
                    c += 0x0800;
                }
                str += libutf8::to_u8string(c);
            }
            unknown_ref += str;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == str);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == str);

            CATCH_REQUIRE(unknown_ref == str);
        }

        CATCH_WHEN("with += & unknown reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref += undefined_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == "");

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == std::string());

            CATCH_REQUIRE(unknown_ref == std::string());
        }

        CATCH_WHEN("with += & self reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref += unknown_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == "");

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == std::string());

            CATCH_REQUIRE(unknown_ref == std::string());

            CATCH_REQUIRE(unknown_ref == unknown_ref);
        }

        CATCH_WHEN("with += & known reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref += reference_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == reference_value);

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == reference_value);

            CATCH_REQUIRE(unknown_ref == reference_value);
        }

        CATCH_WHEN("with += & self reference")
        {
            CATCH_REQUIRE_FALSE(opt.is_defined("unknown"));

            unknown_ref += unknown_ref;

            CATCH_REQUIRE(opt.is_defined("unknown"));

            CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["unknown"] == "");

            std::string const unknown_value(const_cast<advgetopt::getopt const &>(opt)["unknown"]);
            CATCH_REQUIRE(unknown_value == "");

            CATCH_REQUIRE(unknown_ref == "");
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_ref_with_valid_default", "[option_info][valid][reference][long][default]")
{
    CATCH_START_SECTION("No reference on command line, valid default for get_long()")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("test reference.")
                , advgetopt::DefaultValue("459")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: verify references";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "loud",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // verify both parameters the "normal" way
        CATCH_REQUIRE(opt.get_option("reference") != nullptr);
        CATCH_REQUIRE(opt.size("reference") == 0);
        CATCH_REQUIRE(opt.get_string("reference") == "459");

        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.size("verbose") == 1);
        CATCH_REQUIRE(opt.get_string("verbose") == "loud");

        // check the read-only verbose which does not create a reference
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["reference"] == "459");
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["verbose"] == "loud");

        std::string const reference_value(const_cast<advgetopt::getopt const &>(opt)["reference"]);
        CATCH_REQUIRE(reference_value == "459");
        std::string const verbose_value(const_cast<advgetopt::getopt const &>(opt)["verbose"]);
        CATCH_REQUIRE(verbose_value == "loud");

        // get a reference
        advgetopt::option_info_ref reference_ref(opt["reference"]);
        advgetopt::option_info_ref verbose_ref(opt["verbose"]);

        CATCH_REQUIRE(reference_ref.empty());
        CATCH_REQUIRE_FALSE(verbose_ref.empty());

        CATCH_REQUIRE(reference_ref.length() == 3);
        CATCH_REQUIRE(reference_ref.size() == 3);
        CATCH_REQUIRE(verbose_ref.length() == 4);
        CATCH_REQUIRE(verbose_ref.size() == 4);

        CATCH_REQUIRE(reference_ref.get_long() == 459);

        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < reference_ref);
        CATCH_REQUIRE(reference_ref <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > reference_ref);
        CATCH_REQUIRE(reference_ref >= reference_ref);

        CATCH_REQUIRE_FALSE(reference_ref == verbose_ref);
        CATCH_REQUIRE(reference_ref != verbose_ref);
        CATCH_REQUIRE(reference_ref < verbose_ref);
        CATCH_REQUIRE(reference_ref <= verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref > verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref >= verbose_ref);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("option_info_ref_with_invalid_default", "[option_info][invalid][reference][long][default]")
{
    CATCH_START_SECTION("No reference on command line, not valid for get_long()")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("test reference.")
                , advgetopt::DefaultValue("undefined")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: verify references";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "loud",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // verify both parameters the "normal" way
        CATCH_REQUIRE(opt.get_option("reference") != nullptr);
        CATCH_REQUIRE(opt.size("reference") == 0);
        CATCH_REQUIRE(opt.get_string("reference") == "undefined");

        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.size("verbose") == 1);
        CATCH_REQUIRE(opt.get_string("verbose") == "loud");

        // check the read-only verbose which does not create a reference
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["reference"] == "undefined");
        CATCH_REQUIRE(const_cast<advgetopt::getopt const &>(opt)["verbose"] == "loud");

        std::string const reference_value(const_cast<advgetopt::getopt const &>(opt)["reference"]);
        CATCH_REQUIRE(reference_value == "undefined");
        std::string const verbose_value(const_cast<advgetopt::getopt const &>(opt)["verbose"]);
        CATCH_REQUIRE(verbose_value == "loud");

        // get a reference
        advgetopt::option_info_ref reference_ref(opt["reference"]);
        advgetopt::option_info_ref verbose_ref(opt["verbose"]);

        CATCH_REQUIRE(reference_ref.empty());
        CATCH_REQUIRE_FALSE(verbose_ref.empty());

        CATCH_REQUIRE(reference_ref.length() == 9);
        CATCH_REQUIRE(reference_ref.size() == 9);
        CATCH_REQUIRE(verbose_ref.length() == 4);
        CATCH_REQUIRE(verbose_ref.size() == 4);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid default value for a number (undefined) in parameter --reference at offset 0.");
        CATCH_REQUIRE(reference_ref.get_long() == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(reference_ref == reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref != reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref < reference_ref);
        CATCH_REQUIRE(reference_ref <= reference_ref);
        CATCH_REQUIRE_FALSE(reference_ref > reference_ref);
        CATCH_REQUIRE(reference_ref >= reference_ref);

        CATCH_REQUIRE_FALSE(reference_ref == verbose_ref);
        CATCH_REQUIRE(reference_ref != verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref < verbose_ref);
        CATCH_REQUIRE_FALSE(reference_ref <= verbose_ref);
        CATCH_REQUIRE(reference_ref > verbose_ref);
        CATCH_REQUIRE(reference_ref >= verbose_ref);
    }
    CATCH_END_SECTION()
}







// vim: ts=4 sw=4 et
