/*
 * Files:
 *    tests/options_info_ref.cpp
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





CATCH_TEST_CASE("option_info_ref", "[option_info][valid][reference]")
{
    CATCH_START_SECTION("Simple option (verify defaults)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("reference")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
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

        // check the read-only version which does not create a reference
        CATCH_REQUIRE(opt["reference"] == "1001");
        CATCH_REQUIRE(opt["verbose"] == "loud");

        std::string reference_value(opt["reference"]);
        CATCH_REQUIRE(reference_value == "1001");
        std::string verbose_value(opt["verbose"]);
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

        CATCH_REQUIRE(reference_ref + "more" == "zeroloud?more");
        CATCH_REQUIRE("zeroloud?more" == reference_ref + "more");
        CATCH_REQUIRE(reference_ref + std::string("extra") == "zeroloud?extra");
        CATCH_REQUIRE("zeroloud?extra" == reference_ref + std::string("extra"));
        CATCH_REQUIRE(reference_ref + verbose_ref == "zeroloud?loud");
        CATCH_REQUIRE("zeroloud?loud" == reference_ref + verbose_ref);

        CATCH_REQUIRE(reference_ref + '+' == "zeroloud?+");
        CATCH_REQUIRE("zeroloud?+" == reference_ref + '+');
        CATCH_REQUIRE('+' + reference_ref == "+zeroloud?");
        CATCH_REQUIRE("+zeroloud?" == '+' + reference_ref);

        reference_ref = "reset";
        CATCH_REQUIRE('"' + reference_ref + '"' == "\"reset\"");

        reference_ref = verbose_ref;
        CATCH_REQUIRE('(' + reference_ref + ')' == "(loud)");

        std::string const secret("secret");
        reference_ref += ' ';
        reference_ref += secret;
        CATCH_REQUIRE('>' + reference_ref + '<' == ">loud secret<");
        CATCH_REQUIRE(reference_ref + new_value == "loud secretzero");
        CATCH_REQUIRE(new_value + reference_ref == "zeroloud secret");
        CATCH_REQUIRE(reference_ref + " more" == "loud secret more");
        CATCH_REQUIRE("less " + reference_ref == "less loud secret");

        reference_ref = '#';
        CATCH_REQUIRE(reference_ref == "#");

        reference_ref = '\0';
        CATCH_REQUIRE(reference_ref == "");
    CATCH_END_SECTION()
}







// vim: ts=4 sw=4 et
