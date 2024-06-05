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

#include    "advgetopt/exception.h"
#include    "advgetopt/validator_list.h"


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/not_reached.h>
#include    <snapdev/join_strings.h>


// C++
//
#include    <map>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{



namespace
{


typedef std::map<std::string, validator_factory const *>    factory_map_t;

factory_map_t * g_validator_factories;


enum class token_t
{
    TOK_EOF,

    TOK_STRING,
    TOK_IDENTIFIER,
    TOK_REGEX,

    TOK_OPEN_PARENTHESIS,
    TOK_CLOSE_PARENTHESIS,
    TOK_COMMA,
    TOK_OR,

    TOK_INVALID,
};


class token
{
public:
    token(token_t tok, std::string const & value = std::string())
        : f_token(tok)
        , f_value(value)
    {
    }

    token_t tok() const
    {
        return f_token;
    }

    std::string const & value() const
    {
        return f_value;
    }

private:
    token_t         f_token = token_t::TOK_EOF;
    std::string     f_value = std::string();
};

class lexer
{
public:
    lexer(char const * in)
        : f_in(in)
    {
    }

    token next_token()
    {
        for(;;)
        {
            int c(getc());
            switch(c)
            {
            case '\0':
                return token(token_t::TOK_EOF);

            case '(':
                return token(token_t::TOK_OPEN_PARENTHESIS);

            case ')':
                return token(token_t::TOK_CLOSE_PARENTHESIS);

            case ',':
                return token(token_t::TOK_COMMA);

            case '|':
                c = getc();
                if(c != '|')    // allow for || like in C
                {
                    ungetc(c);
                }
                return token(token_t::TOK_OR);

            case '"':
            case '\'':
                {
                    int const quote(c);
                    std::string s;
                    for(;;)
                    {
                        c = getc();
                        if(c == quote)
                        {
                            break;
                        }
                        s += static_cast<char>(c);
                    }
                    return token(token_t::TOK_STRING, s);
                }

            case '/':
                {
                    std::string r;
                    for(;;)
                    {
                        r += static_cast<char>(c);
                        c = getc();
                        if(c == '/')
                        {
                            r += static_cast<char>(c);
                            break;
                        }
                        if(c < ' ' && c != '\t')
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "validator(): unexpected character for a regular expression ("
                                           << static_cast<int>(c)
                                           << ")."
                                           << cppthread::end;
                            return token(token_t::TOK_INVALID);
                        }
                        if(c == '\\')
                        {
                            // we keep the backslash, it's important when
                            // further parsing happens
                            //
                            r += c;

                            c = getc();
                            if(c < ' ' && c != '\t')
                            {
                                cppthread::log << cppthread::log_level_t::error
                                               << "validator(): unexpected escaped character for a regular expression ("
                                               << static_cast<int>(c)
                                               << ")."
                                               << cppthread::end;
                                return token(token_t::TOK_INVALID);
                            }
                        }
                    }
                    // also allow for flags after the closing '/'
                    //
                    // at this time we only support 'i' but here we allow any
                    // letter for forward compatibility
                    //
                    for(;;)
                    {
                        c = getc();
                        if(c == '\0')
                        {
                            break;
                        }
                        if(c < 'a' || c > 'z')
                        {
                            ungetc(c);
                            if(c != ','
                            && c != ')')
                            {
                                cppthread::log << cppthread::log_level_t::error
                                               << "validator(): unexpected flag character for a regular expression ("
                                               << static_cast<int>(c)
                                               << ")."
                                               << cppthread::end;
                                return token(token_t::TOK_INVALID);
                            }
                            break;
                        }
                        r += c;
                    }
                    return token(token_t::TOK_REGEX, r);
                }

            case ' ':
                // ignore spaces
                break;

            default:
                {
                    std::string id;
                    for(;;)
                    {
                        switch(c)
                        {
                        case '(':
                        case ')':
                        case ',':
                        case '|':
                        case ' ':
                            ungetc(c);
                            [[fallthrough]];
                        case '\0':
                            return token(token_t::TOK_IDENTIFIER, id);

                        default:
                            if(c < ' ' || c > '~')
                            {
                                cppthread::log << cppthread::log_level_t::error
                                               << "validator(): unexpected character for an identifier ("
                                               << static_cast<int>(c)
                                               << ")."
                                               << cppthread::end;
                                return token(token_t::TOK_INVALID);
                            }
                            break;

                        }
                        id += static_cast<char>(c);
                        c = getc();
                    }
                }
                break;

            }
        }
        snapdev::NOT_REACHED();
    }

    std::string remains() const
    {
        if(*f_in == '\0')
        {
            return std::string("...EOS");
        }

        return f_in;
    }

private:
    int getc()
    {
        if(f_c != '\0')
        {
            int const c(f_c);
            f_c = '\0';
            return c;
        }

        if(*f_in == '\0')
        {
            return '\0';
        }
        else
        {
            int const c(*f_in);
            ++f_in;
            return c;
        }
    }

    void ungetc(int c)
    {
        if(f_c != '\0')
        {
            throw getopt_logic_error("ungetc() already called once, getc() must be called in between now"); // LCOV_EXCL_LINE
        }
        f_c = c;
    }

    char const *    f_in = nullptr;
    int             f_c = '\0';
};


class validator_with_params
{
public:
    typedef std::vector<validator_with_params>   vector_t;

    validator_with_params(std::string const & name)
        : f_name(name)
    {
    }

    std::string const & get_name() const
    {
        return f_name;
    }

    void add_param(std::string const & param)
    {
        f_params.push_back(param);
    }

    string_list_t const & get_params() const
    {
        return f_params;
    }

private:
    std::string     f_name = std::string();
    string_list_t   f_params = string_list_t();
};


class parser
{
public:
    parser(lexer & l)
        : f_lexer(l)
    {
    }

    bool parse()
    {
        token t(f_lexer.next_token());
        if(t.tok() == token_t::TOK_EOF)
        {
            // empty list
            //
            return true;
        }

        // TODO: show location on an error
        //
        for(;;)
        {
            switch(t.tok())
            {
            case token_t::TOK_REGEX:
                {
                    validator_with_params v("regex");
                    v.add_param(t.value());
                    f_validators.push_back(v);

                    t = f_lexer.next_token();
                }
                break;

            case token_t::TOK_IDENTIFIER:
                {
                    validator_with_params v(t.value());

                    t = f_lexer.next_token();
                    if(t.tok() == token_t::TOK_OPEN_PARENTHESIS)
                    {
                        t = f_lexer.next_token();
                        if(t.tok() != token_t::TOK_CLOSE_PARENTHESIS)
                        {
                            for(;;)
                            {
                                if(t.tok() == token_t::TOK_INVALID)
                                {
                                    return false;
                                }
                                if(t.tok() != token_t::TOK_IDENTIFIER
                                && t.tok() != token_t::TOK_STRING
                                && t.tok() != token_t::TOK_REGEX)
                                {
                                    cppthread::log << cppthread::log_level_t::error
                                                   << "validator(): expected a regex, an identifier or a string inside the () of a parameter. Remaining input: \""
                                                   << f_lexer.remains()
                                                   << "\""
                                                   << cppthread::end;
                                    return false;
                                }
                                v.add_param(t.value());

                                t = f_lexer.next_token();
                                if(t.tok() == token_t::TOK_CLOSE_PARENTHESIS)
                                {
                                    break;
                                }

                                if(t.tok() == token_t::TOK_EOF)
                                {
                                    cppthread::log << cppthread::log_level_t::error
                                                   << "validator(): parameter list must end with ')'. Remaining input: \""
                                                   << f_lexer.remains()
                                                   << "\""
                                                   << cppthread::end;
                                    return false;
                                }

                                if(t.tok() != token_t::TOK_COMMA)
                                {
                                    if(t.tok() == token_t::TOK_INVALID)
                                    {
                                        return false;
                                    }
                                    cppthread::log << cppthread::log_level_t::error
                                                   << "validator(): parameters must be separated by ','. Remaining input: \""
                                                   << f_lexer.remains()
                                                   << "\""
                                                   << cppthread::end;
                                    return false;
                                }
                                do
                                {
                                    t = f_lexer.next_token();
                                }
                                while(t.tok() == token_t::TOK_COMMA);
                            }
                        }
                        t = f_lexer.next_token();
                    }

                    f_validators.push_back(v);
                }
                break;

            default:
                if(t.tok() != token_t::TOK_INVALID)
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << "validator(): unexpected token in validator definition;"
                                      " expected an identifier. Remaining input: \""
                                   << f_lexer.remains()
                                   << "\"."
                                   << cppthread::end;
                }
                return false;

            }

            if(t.tok() == token_t::TOK_EOF)
            {
                return true;
            }

            if(t.tok() != token_t::TOK_OR)
            {
                if(t.tok() != token_t::TOK_INVALID)
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << "validator(): validator definitions must be separated by '|'. Remaining input: \""
                                   << f_lexer.remains()
                                   << "\""
                                   << cppthread::end;
                }
                return false;
            }

            t = f_lexer.next_token();
        }
        snapdev::NOT_REACHED();
    }

    validator_with_params::vector_t const & get_validators() const
    {
        return f_validators;
    }

private:
    lexer &         f_lexer;
    validator_with_params::vector_t
                    f_validators = validator_with_params::vector_t();
};



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
    if(g_validator_factories == nullptr)
    {
        g_validator_factories = new factory_map_t();
    }
    auto it(g_validator_factories->find(factory.get_name()));
    if(it != g_validator_factories->end())
    {
        throw getopt_logic_error(
                  "you have two or more validator factories named \""
                + factory.get_name()
                + "\".");
    }
    (*g_validator_factories)[factory.get_name()] = &factory;
}


validator::pointer_t validator::create(std::string const & name, string_list_t const & data)
{
    if(g_validator_factories == nullptr)
    {
        return validator::pointer_t();  // LCOV_EXCL_LINE
    }

    auto it(g_validator_factories->find(name));
    if(it == g_validator_factories->end())
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

    // the name and parameters can be written as a function call, we have
    // a special case for regex which do not require the function call
    //
    //   validator_list: name_and_params
    //                 | name_and_params ',' validator_list
    //
    //   name_and_params: name '(' params ')'
    //                  | '/' ... '/'              /* regex special case */
    //
    //   name: [a-zA-Z_][a-zA-Z_0-9]*
    //
    //   params: (thing - [,()'" ])
    //         | '\'' (thing - '\'') '\''
    //         | '"' (thing - '"') '"'
    //
    //   thing: [ -~]*
    //        | '\\' [ -~]
    //

    lexer l(name_and_params.c_str());
    parser p(l);
    if(!p.parse())
    {
        return validator::pointer_t();
    }

    validator_with_params::vector_t const & validators(p.get_validators());

    if(validators.size() == 0)
    {
        return validator::pointer_t();
    }

    if(validators.size() == 1)
    {
        return create(validators[0].get_name(), validators[0].get_params());
    }

    // we need a list validator to handle this case
    //
    validator::pointer_t lst(create("list", string_list_t()));
    validator_list::pointer_t list(std::dynamic_pointer_cast<validator_list>(lst));
    if(list == nullptr)
    {
        throw getopt_logic_error("we just created a list and the dynamic cast failed.");    // LCOV_EXCL_LINE
    }
    for(auto const & v : validators)
    {
        list->add_validator(create(v.get_name(), v.get_params()));
    }

    return list;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
