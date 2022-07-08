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

/** \file
 * \brief Implementation of the keywords validator.
 *
 * The keywords validator allows us to check words in a very simple manner
 * (compared to the regular expression validator). It is also likely going
 * to be faster.
 */

// self
//
#include    "advgetopt/validator_keywords.h"


// cppthread
//
#include    <cppthread/log.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_keywords_factory
    : public validator_factory
{
public:
    validator_keywords_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("keywords");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_keywords>(data);
    }
};

validator_keywords_factory         g_validator_keywords_factory;



} // no name namespace





validator_keywords::validator_keywords(string_list_t const & keywords_list)
{
    if(keywords_list.empty())
    {
        cppthread::log << cppthread::log_level_t::error
                       << "validator_keywords() requires at least one parameter."
                       << cppthread::end;
        return;
    }

    std::copy(
          keywords_list.begin()
        , keywords_list.end()
        , std::inserter(f_keywords, f_keywords.begin()));
}


/** \brief Return the name of this validator.
 *
 * This function returns "keywords".
 *
 * \return "keywords".
 */
std::string validator_keywords::name() const
{
    return std::string("keywords");
}


/** \brief Check the value against a regular expression.
 *
 * This function is used to match the value of an argument against a
 * regular expression. It returns true when it does match.
 *
 * \param[in] value  The value to be validated.
 *
 * \return true on a match.
 */
bool validator_keywords::validate(std::string const & value) const
{
    return f_keywords.find(value) != f_keywords.end();
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
