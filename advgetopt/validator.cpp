/*
 * File:
 *    advgetopt/validator.cpp -- advanced get option implementation
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
 *    Doug Barbieri  doug@m2osw.com
 */

/** \file
 * \brief Advanced getopt version functions.
 *
 * The advgetopt environment is versioned. The functions available here
 * give you access to the version, in case you wanted to make sure you
 * had a minimum version or had some special case options when you
 * want to be able to support various versions.
 */

// self
//
#include    "advgetopt/validator.h"


// advgetopt lib
//
#include    "advgetopt/exception.h"
#include    "advgetopt/log.h"


// snapdev lib
//
#include <snapdev/not_used.h>


// boost lib
//
#include <boost/algorithm/string/trim.hpp>


// last include
//
#include <snapdev/poison.h>




namespace advgetopt
{



namespace
{


std::map<std::string, validator_factory const *>      g_validator_factories;


class validator_integer_factory
    : public validator_factory
{
public:
    validator_integer_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("integer");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        snap::NOTUSED(data); // ignore `data`
        return std::make_shared<validator_integer>(data);
    }
};

validator_integer_factory       g_validator_integer_factory;



class validator_regex_factory
    : public validator_factory
{
public:
    validator_regex_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("regex");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_regex>(data);
    }
};

validator_regex_factory     g_validator_regex_factory;



} // no name namespace



/** \brief The destructor to ease derived classes.
 *
 * At this point this destructor does nothing more than help with the
 * virtual table.
 */
validator_factory::~validator_factory()
{
}






/** \brief The validator destructor to support virtuals.
 *
 * This destructor is defined so virtual functions work as expected including
 * the deleter.
 */
validator::~validator()
{
}


/** \fn std::string const & validator::name() const;
 * \brief Return the name of the validator.
 *
 * The name() function is used to get the name of the validator.
 * Validators are recognized by name and added to your options
 * using their name.
 *
 * Note that when an option specifies a validator which it can't find,
 * then an error occurs.
 *
 * \return The name of the validator.
 */


/** \fn bool validator::validate(std::string const & value) const;
 * \brief Return true if \p value validates agains this validator.
 *
 * The function parses the \p value parameter and if it matches the
 * allowed parameters, then it returns true.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */


void validator::register_validator(validator_factory const & factory)
{
    auto it(g_validator_factories.find(factory.get_name()));
    if(it != g_validator_factories.end())
    {
        throw getopt_exception_logic(
                  "you have two or more validator factories named \""
                + factory.get_name()
                + "\".");
    }
    g_validator_factories[factory.get_name()] = &factory;
}


validator::pointer_t validator::create(std::string const & name, string_list_t const & data)
{
    auto it(g_validator_factories.find(name));
    if(it == g_validator_factories.end())
    {
        return validator::pointer_t();
    }

    return it->second->create(data);
}


/** \brief Set the validator for this option.
 *
 * This function parses the specified name and optional parameters and
 * create a corresponding validator for this option.
 *
 * The \p name_and_params string can be defined as:
 *
 * \code
 *     <validator-name>(<param1>, <param2>, ...)
 * \endcode
 *
 * The list of parameters is optional. There may be an empty, just one,
 * or any number of parameters. How the parameters are parsed is left
 * to the validator to decide.
 *
 * If the input string is empty, the current validator, if one is
 * installed, gets removed.
 *
 * \param[in] name_and_params  The validator name and parameters.
 */
validator::pointer_t validator::create(std::string const & name_and_params)
{
    if(name_and_params.empty())
    {
        return validator::pointer_t();
    }

    if(name_and_params.length() >= 2
    && name_and_params[0] == '/')
    {
        // for the regex we have a special case
        //
        string_list_t data{name_and_params};
        return create("regex", data);
    }
    else
    {
        std::string::size_type const params(name_and_params.find('('));
        std::string name(name_and_params);
        string_list_t data;
        if(params != std::string::npos)
        {
            if(name_and_params.back() != ')')
            {
                throw getopt_exception_logic(
                      "invalid validator parameter definition: \""
                    + name_and_params
                    + "\", the ')' is missing.");
            }
            name = name_and_params.substr(0, params);
            split_string(name_and_params.substr(params + 1, name_and_params.length() - params - 2)
                       , data
                       , {","});
        }
        return create(name, data);
    }
}






/** \brief Initialize the integer validator.
 *
 * The constructor accepts a string with values and ranges which are
 * used to limit the values that can be used with this parameter.
 *
 * Remember that the default does not have to be included in these
 * values. It will still be viewed as \em valid.
 *
 * The string uses the following format:
 *
 * \code
 *    start: range
 *         | start ',' range
 *
 *    range: number
 *         | number '...' number
 *
 *    number: [-+]?[0-9]+
 * \endcode
 *
 * Note that a single number is considered to be a range and is managed
 * the exact same way. A value which matches any of the ranges is considered
 * valid.
 *
 * Example:
 *
 * \code
 *     "-100...100,-1000"
 * \endcode
 *
 * This example allows all values between -100 and +100 inclusive and also
 * allows the value -1000.
 *
 * \param[in] ranges  The ranges used to limit the integer.
 */
validator_integer::validator_integer(string_list_t const & range_list)
{
    range_t range;
    for(auto r : range_list)
    {
        std::string::size_type const pos(r.find("..."));
        if(pos == std::string::npos)
        {
            if(!convert_string(r, range.f_minimum))
            {
                log << log_level_t::error
                    << r
                    << " is not a valid value for your ranges;"
                       " it must only digits, optionally preceeded by a sign (+ or -)"
                       " and not overflow an int64_t value."
                    << end;
                continue;
            }
            range.f_maximum = range.f_minimum;
        }
        else
        {
            std::string min_value(r.substr(0, pos));
            boost::trim(min_value);
            if(!convert_string(min_value, range.f_minimum))
            {
                log << log_level_t::error
                    << min_value
                    << " is not a valid value for your ranges;"
                       " it must only digits, optionally preceeded by a sign (+ or -)"
                       " and not overflow an int64_t value."
                    << end;
                continue;
            }

            std::string max_value(r.substr(pos + 3));
            boost::trim(max_value);
            if(!convert_string(max_value, range.f_maximum))
            {
                log << log_level_t::error
                    << max_value
                    << " is not a valid value for your ranges;"
                       " it must only digits, optionally preceeded by a sign (+ or -)"
                       " and not overflow an int64_t value."
                    << end;
                continue;
            }

            if(range.f_minimum > range.f_maximum)
            {
                log << log_level_t::error
                    << min_value
                    << " has to be smaller or equal to "
                    << max_value
                    << "; you have an invalid range."
                    << end;
                continue;
            }
        }
        f_allowed_values.push_back(range);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "integer".
 *
 * \return "integer".
 */
std::string const validator_integer::name() const
{
    return std::string("integer");
}


/** \brief Determine whether value is an integer.
 *
 * This function verifies that the specified value is a valid integer.
 *
 * It makes sures that the value is only composed of digits (`[0-9]+`).
 * It may also start with a sign (`[-+]?`).
 *
 * The function also makes sure that the value fits in an `int64_t` value.
 *
 * \todo
 * Add support for binary, octal, hexadecimal.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */
bool validator_integer::validate(std::string const & value) const
{
    std::int64_t result(0);
    if(convert_string(value, result))
    {
        if(f_allowed_values.empty())
        {
            return true;
        }

        for(auto f : f_allowed_values)
        {
            if(result >= f.f_minimum
            && result <= f.f_maximum)
            {
                return true;
            }
        }
        return false;
    }

    return false;
}


/** \brief Convert a string to an std::int64_t value.
 *
 * This function is used to convert a string to an integer with full
 * boundary verification.
 *
 * The result can also get checked against ranges as defined in the
 * constructor.
 *
 * \return true if the conversion succeeded.
 */
bool validator_integer::convert_string(std::string const & value, std::int64_t & result)
{
    std::uint64_t integer(0);
    char const * s(value.c_str());

    char sign('\0');
    if(*s == '-' || *s == '+')
    {
        sign = *s;
        ++s;
    }

    if(*s == '\0')
    {
        // empty string, not considered valid
        //
        return false;
    }

    for(;;)
    {
        char const c(*s++);
        if(c == '\0')
        {
            // valid
            //
            if(sign == '-')
            {
                if(integer > 0x8000000000000000ULL)
                {
                    return false;
                }
                result = -integer;
            }
            else
            {
                if(integer > 0x7FFFFFFFFFFFFFFFULL)
                {
                    return false;
                }
                result = integer;
            }
            return true;
        }
        if(c < '0' || c > '9')
        {
            // invalid digit
            //
            return false;
        }

        std::uint64_t const old(integer);
        integer = integer * 10 + c - '0';
        if(integer < old)
        {
            // overflow
            //
            return false;
        }
    }
}










validator_regex::validator_regex(string_list_t const & regex_list)
{
    if(regex_list.size() > 1)
    {
        log << log_level_t::error
            << "validator_regex() only supports one parameter; "
            << regex_list.size()
            << " were supplied; single or double quotation may be required?"
            << end;
        return;
    }

    std::string regex;
    if(!regex_list.empty())
    {
        regex = regex_list[0];
    }
    std::regex::flag_type flags =  std::regex_constants::extended;
    if(regex.length() >= 2
    && regex[0] == '/')
    {
        auto it(regex.end());
        for(--it; it != regex.begin(); --it)
        {
            if(*it == '/')
            {
                break;
            }
            switch(*it)
            {
            case 'i':
                flags |= std::regex_constants::icase;
                break;

            default:
                log << log_level_t::error
                    << "unsupported regex flag "
                    << *it
                    << " in regular expression \""
                    << regex
                    << "\"."
                    << end;
                break;

            }
        }
        if(it == regex.begin())
        {
            log << log_level_t::error
                << "invalid regex definition, ending / is missing in \""
                << regex
                << "\"."
                << end;

            f_regex = std::regex(std::string(regex.begin() + 1, regex.end()), flags);
        }
        else
        {
            f_regex = std::regex(std::string(regex.begin() + 1, it), flags);
        }
    }
    else
    {
        f_regex = std::regex(regex, flags);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "regex".
 *
 * \return "regex".
 */
std::string const validator_regex::name() const
{
    return std::string("regex");
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
bool validator_regex::validate(std::string const & value) const
{
    std::smatch info;
    return std::regex_match(value, info, f_regex);
}





} // namespace advgetopt
// vim: ts=4 sw=4 et
