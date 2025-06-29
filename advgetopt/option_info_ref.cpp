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


/** \file
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to define one command
 * line option.
 */

// self
//
#include    "advgetopt/option_info.h"

#include    "advgetopt/exception.h"
#include    "advgetopt/validator_double.h"
#include    "advgetopt/validator_integer.h"


// cppthread
//
#include    <cppthread/log.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// last include
//
#include    <snapdev/poison.h>



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
 * \note
 * The option may not yet be defined in which case the default value is
 * used as the current value.
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
 * \note
 * If the value is not currently defined, this function returns the
 * length of the default value.
 *
 * \return The length of the option's value.
 */
bool option_info_ref::empty() const
{
    if(f_opt->is_defined())
    {
        return f_opt->get_value().empty();
    }

    return true;
}


/** \brief Return the length of the option's value.
 *
 * This function returns the length of the option's value. This is the
 * number of bytes in the string.
 *
 * \note
 * If the value is not currently defined, this function returns the
 * length of the default value.
 *
 * \return The length of the option's value.
 */
size_t option_info_ref::length() const
{
    if(f_opt->is_defined())
    {
        return f_opt->get_value().length();
    }

    return f_opt->get_default().length();
}


/** \brief Return the length of the option's value.
 *
 * This function returns the length of the option's value. This is the
 * number of bytes in the string.
 *
 * \return The length of the option's value.
 */
size_t option_info_ref::size() const
{
    return length();
}


/** \brief Retrieve the referenced option as a long.
 *
 * This function attempts to retrieve the option value as a long integer.
 *
 * If the value is not yet defined, the function attempts to return the
 * default value converted to an integer. If that fails, the function
 * returns -1 after it emitted an error in the log.
 *
 * When the value is not defined and there is no default, the function
 * returns 0 (as if an empty string represented 0.)
 *
 * \return The value as a long or -1 or 0.
 */
long option_info_ref::get_long() const
{
    if(f_opt->is_defined())
    {
        return f_opt->get_long();
    }

    if(!f_opt->has_default())
    {
        return 0;
    }

    std::int64_t v;
    if(!validator_integer::convert_string(f_opt->get_default(), v))
    {
        cppthread::log << cppthread::log_level_t::error
                       << "invalid default value for a number ("
                       << f_opt->get_default()
                       << ") in parameter --"
                       << f_opt->get_name()
                       << " at offset 0."
                       << cppthread::end;
        return -1;
    }

    return v;
}


/** \brief Retrieve the referenced option as a double.
 *
 * This function attempts to retrieve the option value as a double floating
 * point.
 *
 * If the value is not yet defined, the function attempts to return the
 * default value converted to a double. If that fails, the function
 * returns -1 after it emitted an error in the log.
 *
 * When the value is not defined and there is no default, the function
 * returns 0 (as if an empty string represented 0.)
 *
 * \return The value as a double or -1.0 or 0.0.
 */
double option_info_ref::get_double() const
{
    if(f_opt->is_defined())
    {
        return f_opt->get_double();
    }

    if(!f_opt->has_default())
    {
        return 0.0;
    }

    double v;
    if(!validator_double::convert_string(f_opt->get_default(), v))
    {
        cppthread::log << cppthread::log_level_t::error
                       << "invalid default value as a double number ("
                       << f_opt->get_default()
                       << ") in parameter --"
                       << f_opt->get_name()
                       << " at offset 0."
                       << cppthread::end;
        return -1.0;
    }

    return v;
}


/** \brief Convert the reference to a string (a.k.a. read the value)
 *
 * This cast operator transforms the reference in a string which has
 * the contents of the option value.
 *
 * \return The option contents as a string.
 */
option_info_ref::operator std::string () const
{
    if(f_opt->is_defined())
    {
        return f_opt->get_value();
    }

    return f_opt->get_default();
}


/** \brief Set the option value to \p value.
 *
 * This assignment operator is used to change the value of the option.
 *
 * The input character is transform in a string and saved as such in the
 * option. If the character is '\0', then the value is cleared instead.
 *
 * \param[in] value  The ISO-8859-1 character to save in the option_info object.
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
    f_opt->set_value(0, v); // source considered DIRECT
    return *this;
}


/** \brief Set the option value to \p value.
 *
 * This assignment operator is used to change the value of the option.
 *
 * The input character is transform in a string and saved as such in the
 * option. If the character is '\0', then the value is cleared instead.
 *
 * \param[in] value  The unicode character to save in the option_info object.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator = (char32_t value)
{
    std::string v;
    if(value != U'\0')
    {
        v = libutf8::to_u8string(value);
    }
    f_opt->set_value(0, v); // source considered DIRECT
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
    if(value == nullptr)
    {
        f_opt->set_value(0, std::string()); // source considered DIRECT
    }
    else
    {
        f_opt->set_value(0, value); // source considered DIRECT
    }
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
    f_opt->set_value(0, value); // source considered DIRECT
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
    f_opt->set_value(0, value); // source considered DIRECT
    return *this;
}


/** \brief Append the character \p value to this option's value.
 *
 * This assignment operator can be used to append a character to the
 * existing value of the option.
 *
 * \note
 * The character is taken as an ISO-8859-1. If you want to use a Unicode
 * character, make sure to use a char32_t character.
 *
 * \param[in] value  The character to append to the option_info's value.
 *
 * \return A reference to this object_info_ref.
 */
option_info_ref & option_info_ref::operator += (char value)
{
    std::string v;
    if(value != '\0')
    {
        v += value;
    }
    f_opt->set_value(0, static_cast<std::string>(*this) + v); // source considered DIRECT
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
option_info_ref & option_info_ref::operator += (char32_t value)
{
    std::string v;
    if(value != '\0')
    {
        v += libutf8::to_u8string(value);
    }
    f_opt->set_value(0, static_cast<std::string>(*this) + v); // source considered DIRECT
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
    std::string v;
    if(value != nullptr)
    {
        v = value;
    }
    f_opt->set_value(0, static_cast<std::string>(*this) + v); // source considered DIRECT
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
    f_opt->set_value(0, static_cast<std::string>(*this) + value); // source considered DIRECT
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
    f_opt->set_value( // source considered DIRECT
              0
            , static_cast<std::string>(*this) + static_cast<std::string>(value));
    return *this;
}


/** \brief Append the character \p value to this option's value.
 *
 * This operator is used to append a character to the value of the option
 * and returns the result as a string.
 *
 * \note
 * This version appends an ISO-8859-1 character. Make sure to use a
 * char32_t character to add a Unicode character.
 *
 * \param[in] value  The character to append to the option_info's value.
 *
 * \return A string with the resulting concatenation.
 */
std::string option_info_ref::operator + (char value) const
{
    return static_cast<std::string>(*this) + value;
}


/** \brief Append the character \p value to this option's value.
 *
 * This operator is used to append a Unicode character to the value of the
 * option and returns the result as a string.
 *
 * \param[in] value  The character to append to the option_info's value.
 *
 * \return A string with the resulting concatenation.
 */
std::string option_info_ref::operator + (char32_t value) const
{
    return static_cast<std::string>(*this) + libutf8::to_u8string(value);
}


/** \brief Append \p value to this option's value.
 *
 * This operator is used to append a string to the value of the option
 * and return the result as a string.
 *
 * \param[in] value  The string to append to the option_info's value.
 *
 * \return A string with the resulting concatenation.
 */
std::string option_info_ref::operator + (char const * value) const
{
    if(value == nullptr)
    {
        return *this;
    }
    return static_cast<std::string>(*this) + value;
}


/** \brief Append \p value to this option's value.
 *
 * This operator is used to append a string to the value of the option
 * and return the result as a string.
 *
 * \param[in] value  The value to append to the option_info's value.
 *
 * \return A string with the resulting concatenation.
 */
std::string option_info_ref::operator + (std::string const & value) const
{
    return static_cast<std::string>(*this) + value;
}


/** \brief Append the value of this \p value option to this option_info's value.
 *
 * This operator is used to append two option references to each others and
 * return the concatenated string as the result.
 *
 * \param[in] value  The other option to read the value from.
 *
 * \return A reference to this object_info_ref.
 */
std::string option_info_ref::operator + (option_info_ref const & value) const
{
    return static_cast<std::string>(*this)
         + static_cast<std::string>(value);
}


/** \brief Concatenate a character and an option reference value.
 *
 * This operator concatenates the \p value ISO-8859-1 character to the front
 * of the \p rhs reference.
 *
 * \param[in] value  A character to add to the left of the referred value.
 * \param[in] rhs  The referred value.
 *
 * \return The concatenated result.
 */
std::string operator + (char value, option_info_ref const & rhs)
{
    return value + static_cast<std::string>(rhs);
}


/** \brief Concatenate a character and an option reference value.
 *
 * This operator concatenates the \p value Unicode character to the front
 * of the \p rhs reference.
 *
 * \param[in] value  A character to add to the left of the referred value.
 * \param[in] rhs  The referred value.
 *
 * \return The concatenated result.
 */
std::string operator + (char32_t value, option_info_ref const & rhs)
{
    return libutf8::to_u8string(value) + static_cast<std::string>(rhs);
}


/** \brief Concatenate a string and an option reference value.
 *
 * This operator concatenates the \p value string to the front
 * of the \p rhs reference.
 *
 * \param[in] value  A character to add to the left of the referred value.
 * \param[in] rhs  The referred value.
 *
 * \return The concatenated result.
 */
std::string operator + (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return rhs;
    }
    return value + static_cast<std::string>(rhs);
}


/** \brief Concatenate a string and an option reference value.
 *
 * This operator concatenates the \p value string to the front
 * of the \p rhs reference.
 *
 * \param[in] value  A character to add to the left of the referred value.
 * \param[in] rhs  The referred value.
 *
 * \return The concatenated result.
 */
std::string operator + (std::string const & value, option_info_ref const & rhs)
{
    return value + static_cast<std::string>(rhs);
}


/** \brief Check whether the value is an empty string or not.
 *
 * This function calls the empty function and returns the opposite result.
 *
 * \return true if the value is not an empty string.
 */
option_info_ref::operator bool () const
{
    return !empty();
}


/** \brief Check whether the value is an empty string or not.
 *
 * This function calls the empty function and returns the result.
 *
 * \return true if the value is an empty string.
 */
bool option_info_ref::operator ! () const
{
    return empty();
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
    if(value == nullptr)
    {
        return empty();
    }
    return static_cast<std::string>(*this) == value;
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
    return static_cast<std::string>(*this) == value;
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
    return static_cast<std::string>(*this) == static_cast<std::string>(value);
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
    if(value == nullptr)
    {
        return !empty();
    }
    return static_cast<std::string>(*this) != value;
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
    return static_cast<std::string>(*this) != value;
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
    return static_cast<std::string>(*this) != static_cast<std::string>(value);
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
    if(value == nullptr)
    {
        return false;
    }
    return static_cast<std::string>(*this) < value;
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
    return static_cast<std::string>(*this) < value;
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
    return static_cast<std::string>(*this) < static_cast<std::string>(value);
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
    if(value == nullptr)
    {
        return empty();
    }
    return static_cast<std::string>(*this) <= value;
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
    return static_cast<std::string>(*this) <= value;
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
    return static_cast<std::string>(*this) <= static_cast<std::string>(value);
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
    if(value == nullptr)
    {
        return !empty();
    }
    return static_cast<std::string>(*this) > value;
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
    return static_cast<std::string>(*this) > value;
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
    return static_cast<std::string>(*this) > static_cast<std::string>(value);
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
    if(value == nullptr)
    {
        return true;
    }
    return static_cast<std::string>(*this) >= value;
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
    return static_cast<std::string>(*this) >= value;
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
    return static_cast<std::string>(*this) >= static_cast<std::string>(value);
}



/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is equal to rhs.
 */
bool operator == (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return rhs.empty();
    }
    return value == static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is equal to rhs.
 */
bool operator == (std::string const & value, option_info_ref const & rhs)
{
    return value == static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is not equal to rhs.
 */
bool operator != (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return !rhs.empty();
    }
    return value != static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is not equal to rhs.
 */
bool operator != (std::string const & value, option_info_ref const & rhs)
{
    return value != static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered smaller than rhs.
 */
bool operator < (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return !rhs.empty();
    }
    return value < static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered smaller than rhs.
 */
bool operator < (std::string const & value, option_info_ref const & rhs)
{
    return value < static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered smaller or equal to rhs.
 */
bool operator <= (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return true;
    }
    return value <= static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered smaller or equal to rhs.
 */
bool operator <= (std::string const & value, option_info_ref const & rhs)
{
    return value <= static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered larger than rhs.
 */
bool operator > (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return false;
    }
    return value > static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered larger than rhs.
 */
bool operator > (std::string const & value, option_info_ref const & rhs)
{
    return value > static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered larger or equal to rhs.
 */
bool operator >= (char const * value, option_info_ref const & rhs)
{
    if(value == nullptr)
    {
        return rhs.empty();
    }
    return value >= static_cast<std::string>(rhs);
}


/** \brief Compare \p value with the value of the right hand-side option.
 *
 * This operator compares the specified \p value with the value of the
 * option specified as the \p rhs (right hand-side.)
 *
 * \param[in] value  A string to compare an option's value with.
 * \param[in] rhs  The option to compare against \p value.
 *
 * \return true if value is considered larger or equal to rhs.
 */
bool operator >= (std::string const & value, option_info_ref const & rhs)
{
    return value >= static_cast<std::string>(rhs);
}



}   // namespace advgetopt



// vim: ts=4 sw=4 et
