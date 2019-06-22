/*
 * File:
 *    advgetopt/option_info_ref.cpp -- a replacement to the Unix getopt() implementation
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
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to define one command
 * line option.
 */

// self
//
#include "advgetopt/option_info.h"


// advgetopt lib
//
#include "advgetopt/exception.h"
#include "advgetopt/log.h"


// libutf8 lib
//
#include    <libutf8/libutf8.h>


// boost lib
//
#include <boost/algorithm/string/replace.hpp>


// last include
//
#include <snapdev/poison.h>




namespace advgetopt
{




/** \brief Initialize a reference to an option_info object.
 *
 * This constructor creates a reference to the specified \p opt
 * option_info object.
 *
 * This gives you read and write access to the very first value held by
 * the \p opt object.
 *
 * \param[in] opt  The option to create the reference of.
 */
option_info_ref::option_info_ref(option_info::pointer_t opt)
    : f_opt(opt)
{
}


/** \brief Retrieve the length of the option's value.
 *
 * This function checks the option's value and returns true if it is empty.
 *
 * \return The length of the option's value.
 */
bool option_info_ref::empty() const
{
    return f_opt->get_value().empty();
}


/** \brief Return the length of the option's value.
 *
 * This function returns the length of the option's value.
 *
 * \return The length of the option's value.
 */
size_t option_info_ref::length() const
{
    return f_opt->get_value().length();
}


/** \brief Return the length of the option's value.
 *
 * This function returns the length of the option's value.
 *
 * \return The length of the option's value.
 */
size_t option_info_ref::size() const
{
    return f_opt->get_value().length();
}


/** \brief Convert the reference to a string (a.k.a. read the value)
 *
 * This cast operator transforms the reference in a string which has
 * the contents of the option value.
 */
option_info_ref::operator std::string () const
{
    return f_opt->get_value();
}


/** \brief Set the option value to \p value.
 *
 * This assignment operator is used to change the value of the option.
 *
 * The input character is transform in a string and saved as such in the
 * option. If the character is '\0', then the value is cleared instead.
 *
 * \param[in] value  The new value to save in the option_info object.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator = (char value)
{
    std::string v;
    if(value != '\0')
    {
        v += value;
    }
    f_opt->set_value(0, v);
    return *this;
}


/** \brief Set the option value to \p value.
 *
 * This assignment operator can be used to change the value of the
 * option.
 *
 * \param[in] value  The new value to save in the option_info object.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator = (char const * value)
{
    f_opt->set_value(0, value);
    return *this;
}


/** \brief Set the option value to \p value.
 *
 * This assignment operator can be used to change the value of the
 * option.
 *
 * \param[in] value  The new value to save in the option_info object.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator = (std::string const & value)
{
    f_opt->set_value(0, value);
    return *this;
}


/** \brief Set the value of this option to the value of another option.
 *
 * This assignment operator allows you to copy the value from reference
 * value to another.
 *
 * \param[in] value  The other option to read the value from.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator = (option_info_ref const & value)
{
    f_opt->set_value(0, value.f_opt->get_value());
    return *this;
}


/** \brief Append the character \p value to this option's value.
 *
 * This assignment operator can be used to append a character to the
 * existing value of the option.
 *
 * \param[in] value  The character to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator += (char value)
{
    f_opt->set_value(0, f_opt->get_value() + value);
    return *this;
}


/** \brief Append \p value to this option's value.
 *
 * This assignment operator can be used to append a string to the existing
 * value of the option.
 *
 * \param[in] value  The value to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator += (char const * value)
{
    f_opt->set_value(0, f_opt->get_value() + value);
    return *this;
}


/** \brief Append \p value to this option's value.
 *
 * This assignment operator is used to append a string to the existing
 * value of the option.
 *
 * \param[in] value  The value to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator += (std::string const & value)
{
    f_opt->set_value(0, f_opt->get_value() + value);
    return *this;
}


/** \brief Append the value of this \p value option to this option_info's value.
 *
 * This assignment operator is used to append a string to the existing
 * value of this option.
 *
 * \param[in] value  The other option to read the value from.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator += (option_info_ref const & value)
{
    f_opt->set_value(0, f_opt->get_value() + value.f_opt->get_value());
    return *this;
}


/** \brief Append the character \p value to this option's value.
 *
 * This operator is used to append a character to the value of the option
 * and returns the result as a string.
 *
 * \param[in] value  The character to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
std::string option_info_ref::operator + (char value) const
{
    return f_opt->get_value() + value;
}


/** \brief Append \p value to this option's value.
 *
 * This operator is used to append a string to the value of the option
 * and return the result as a string.
 *
 * \param[in] value  The string to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
std::string option_info_ref::operator + (char const * value) const
{
    return f_opt->get_value() + value;
}


/** \brief Append \p value to this option's value.
 *
 * This operator is used to append a string to the value of the option
 * and return the result as a string.
 *
 * \param[in] value  The value to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
std::string option_info_ref::operator + (std::string const & value) const
{
    return f_opt->get_value() + value;
}


/** \brief Append the value of this \p value option to this option_info's value.
 *
 * This assignment operator is used to append a string to the existing
 * value of this option.
 *
 * \param[in] value  The other option to read the value from.
 *
 * \return A reference to this object_info_ref.
 */
std::string option_info_ref::operator + (option_info_ref const & value) const
{
    return f_opt->get_value() + value.f_opt->get_value();
}


std::string operator + (char value, option_info_ref const & rhs)
{
    return value + rhs.f_opt->get_value();
}


std::string operator + (char const * value, option_info_ref const & rhs)
{
    return value + rhs.f_opt->get_value();
}


std::string operator + (std::string const & value, option_info_ref const & rhs)
{
    return value + rhs.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are equal.
 */
bool option_info_ref::operator == (char const * value) const
{
    return f_opt->get_value() == value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are equal.
 */
bool option_info_ref::operator == (std::string const & value) const
{
    return f_opt->get_value() == value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are equal.
 */
bool option_info_ref::operator == (option_info_ref const & value) const
{
    return f_opt->get_value() == value.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are not equal.
 */
bool option_info_ref::operator != (char const * value) const
{
    return f_opt->get_value() != value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are not equal.
 */
bool option_info_ref::operator != (std::string const & value) const
{
    return f_opt->get_value() != value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if both are not equal.
 */
bool option_info_ref::operator != (option_info_ref const & value) const
{
    return f_opt->get_value() != value.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less than rhs.
 */
bool option_info_ref::operator < (char const * value) const
{
    return f_opt->get_value() < value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less than rhs.
 */
bool option_info_ref::operator < (std::string const & value) const
{
    return f_opt->get_value() < value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less than rhs.
 */
bool option_info_ref::operator < (option_info_ref const & value) const
{
    return f_opt->get_value() < value.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less or equal than rhs.
 */
bool option_info_ref::operator <= (char const * value) const
{
    return f_opt->get_value() <= value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less or equal than rhs.
 */
bool option_info_ref::operator <= (std::string const & value) const
{
    return f_opt->get_value() <= value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is less or equal than rhs.
 */
bool option_info_ref::operator <= (option_info_ref const & value) const
{
    return f_opt->get_value() <= value.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater than rhs.
 */
bool option_info_ref::operator > (char const * value) const
{
    return f_opt->get_value() > value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater than rhs.
 */
bool option_info_ref::operator > (std::string const & value) const
{
    return f_opt->get_value() > value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater than rhs.
 */
bool option_info_ref::operator > (option_info_ref const & value) const
{
    return f_opt->get_value() > value.f_opt->get_value();
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater or equal than rhs.
 */
bool option_info_ref::operator >= (char const * value) const
{
    return f_opt->get_value() >= value;
}


/** \brief Compare this option's value with the specified string.
 *
 * This operator compares this option's value with the specified string.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater or equal than rhs.
 */
bool option_info_ref::operator >= (std::string const & value) const
{
    return f_opt->get_value() >= value;
}


/** \brief Compare this option's value with the value of option \p value.
 *
 * This operator compares this option's value with the value of the
 * option specified in \p value.
 *
 * \param[in] value  A string to compare this option's value with.
 *
 * \return true if lhs is greater or equal than rhs.
 */
bool option_info_ref::operator >= (option_info_ref const & value) const
{
    return f_opt->get_value() >= value.f_opt->get_value();
}



bool operator == (char const * value, option_info_ref const & rhs)
{
    return value == rhs.f_opt->get_value();
}


bool operator == (std::string const & value, option_info_ref const & rhs)
{
    return value == rhs.f_opt->get_value();
}


bool operator != (char const * value, option_info_ref const & rhs)
{
    return value != rhs.f_opt->get_value();
}


bool operator != (std::string const & value, option_info_ref const & rhs)
{
    return value != rhs.f_opt->get_value();
}


bool operator < (char const * value, option_info_ref const & rhs)
{
    return value < rhs.f_opt->get_value();
}


bool operator < (std::string const & value, option_info_ref const & rhs)
{
    return value < rhs.f_opt->get_value();
}


bool operator <= (char const * value, option_info_ref const & rhs)
{
    return value <= rhs.f_opt->get_value();
}


bool operator <= (std::string const & value, option_info_ref const & rhs)
{
    return value <= rhs.f_opt->get_value();
}


bool operator > (char const * value, option_info_ref const & rhs)
{
    return value > rhs.f_opt->get_value();
}


bool operator > (std::string const & value, option_info_ref const & rhs)
{
    return value > rhs.f_opt->get_value();
}


bool operator >= (char const * value, option_info_ref const & rhs)
{
    return value >= rhs.f_opt->get_value();
}


bool operator >= (std::string const & value, option_info_ref const & rhs)
{
    return value >= rhs.f_opt->get_value();
}





}   // namespace advgetopt
// vim: ts=4 sw=4 et
