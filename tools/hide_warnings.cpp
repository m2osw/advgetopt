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
 * \brief Tool used to hide "Gtk-warning" messages from terminal.
 *
 * This tool can be used to hide certain errors and warnings from your
 * console. Many of us really do not care about those Gtk-WARNINGS, which
 * we cannot really do anything about, except parse out with such a tool.
 *
 * To use, create an alias in your ~/.bashrc file:
 *
 * \code
 * alias gvim="hide-warnings gvim"
 * alias meld="hide-warnings meld"
 * ...any command that generates Gtk-WARNINGS...
 * \endcode
 *
 * If you want to parse out other things, you may change the default regex
 * ('gtk-wanring|gtk-critical|glib-gobject-warning|^$') with whatever you
 * want. Use the --regex command line option for that purpose:
 *
 * \code
 * alias gimp="hide-warnings --regex 'cannot change name of operation class|glib-gobject-warning|gtk-warning|^$' gimp"
 * \endcode
 *
 * If your command starts with a dash (-), then use -- on the command
 * line before your command:
 *
 * \code
 * alias weird="hide-warnings --regex 'forget|that' -- -really-weird'
 * \endcode
 *
 * \todo
 * Transform the command line handling to make use of the advgetopt. I wrote
 * this tool in a separate project at first but wanted to share it and
 * make it an example on how to use advgetopt but as we can see at the moment
 * it's not yet converted... For now, it's a C++ file at least.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// C++
//
#include    <iostream>


// C
//
#include    <unistd.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <fcntl.h>
#include    <limits.h>
#include    <errno.h>
#include    <regex.h>
#include    <poll.h>



char const * const  g_version = "1.0";
char const * const  g_default_regex = "gtk-warning|gtk-critical|glib-gobject-warning|^$";

char const *        g_progname = NULL;
char const *        g_regex = NULL;
int                 g_case_sensitive = 0;
int                 g_filter_stdout = 0;

#define IN_OUT_BUFSIZ       (64 * 1024)
struct io_buf
{
    size_t              f_pos = 0;
    char                f_buf[IN_OUT_BUFSIZ + 1] = { 0 };
};
struct io_buf       g_buf_out;
struct io_buf       g_buf_err;


void usage()
{
    std::cout << "Usage: " << g_progname << " [--opts] command [cmd-opts]" << std::endl
              << "Where --opts is one or more of:" << std::endl
              << "   --help    | -h           print out this help screen" << std::endl
              << "   --version | -V           print out the version of " << g_progname << std::endl
              << "   --regex   | -r 'regex'   regex of messages to hide" << std::endl
              << "   --case    | -c           make the regex case sensitive" << std::endl
              << "   --out                    also filter stdout" << std::endl
              << "   --                       end list of " << g_progname << " options\n" << std::endl
              << "And where command and [cmd-opts] is the command to execute and its options." << std::endl;
    exit(0);
}



void output_data(FILE * out, regex_t const * regex, char * str, size_t len)
{
    int saved_errno, r;
    size_t sz;
    char save_eol;

    if(regex != NULL)
    {
        /* run the regex without the "\n" included */
        sz = len > 0 && str[len - 1] == '\n' ? len - 1 : len;
        save_eol = str[sz];
        str[sz] = '\0';
        r = regexec(regex, str, 0, NULL, 0);
        str[sz] = save_eol;
        if(r == 0)
        {
            /* the pattern matched, the user does not want to see that one */
            return;
        }
    }

    sz = fwrite(str, len, 1, out);
    if(sz != 1)
    {
        saved_errno = errno;
        std::cerr << std::endl
                  << g_progname << ":error: write() to stdout/stderr failed: "
                  << strerror(saved_errno)
                  << ". ("
                  << sz
                  << ")";
        exit(1);
    }
}


void read_pipe(int pipe, FILE * out, regex_t const * regex, struct io_buf * io)
{
    ssize_t sz;

    for(;;)
    {
        /* read some data */
        sz = read(pipe, io->f_buf + io->f_pos, IN_OUT_BUFSIZ - io->f_pos);
        if(sz <= 0)
        {
            if(sz < 0
            && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << g_progname
                          << ":error: read() of stdout pipe failed."
                          << std::endl;
                exit(1);
            }
            return;
        }

        /* got some data, search for a "\n" */
        for(; sz > 0; --sz)
        {
            if(io->f_buf[io->f_pos] == '\n')
            {
                /* write that to the output, including the "\n" */
                ++io->f_pos;
                output_data(out, regex, io->f_buf, io->f_pos);
                memmove(io->f_buf, io->f_buf + io->f_pos, sz);
                io->f_pos = 0;
            }
            else
            {
                ++io->f_pos;
            }
        }
        if(io->f_pos >= IN_OUT_BUFSIZ)
        {
            /* our buffer is full, we must empty it... (but it should
             * be rare that a normal process outputs a string of over 64Kb
             * without at least one "\n" character!)
             */
            output_data(out, regex, io->f_buf, IN_OUT_BUFSIZ);
            io->f_pos = 0;
        }
    }
}


int main(int argc, char * argv[], char * envp[])
{
    int i, pipe_out[2], pipe_err[2], child_pid, saved_errno;
    size_t j, len, cmd_len;
    char * path, * p, * e, * n;
    struct pollfd fds[2];
    nfds_t count;
    regex_t regex;

    /* get the basename from argv[0] */
    g_progname = strrchr(argv[0], '/');
    if(g_progname == NULL)
    {
        g_progname = argv[0];
    }
    else
    {
        /* skip the '/' */
        ++g_progname;
    }

    /* if there are some parameters that start with '-' or '--'
     * before a parameter without such, then these are command line
     * options to hide-warnings
     */

    g_regex = g_default_regex;

    for(i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            /* long option? */
            if(argv[i][1] == '-')
            {
                if(argv[i][2] == '\0')
                {
                    /* we found a "--" */
                    break;
                }

                if(strcmp(argv[i] + 2, "help") == 0)
                {
                    usage();
                    /*NOTREACHED*/
                }
                if(strcmp(argv[i] + 2, "version") == 0)
                {
                    std::cout << g_version << std::endl;
                    exit(0);
                    /*NOTREACHED*/
                }
                if(strcmp(argv[i] + 2, "regex") == 0)
                {
                    ++i;
                    if(i >= argc)
                    {
                        std::cerr << g_progname
                                  << ":error: --regex must be followed by a regular expression."
                                  << std::endl;
                        exit(1);
                    }
                    g_regex = argv[i];
                }
                else if(strncmp(argv[i] + 2, "regex=", 6) == 0)
                {
                    g_regex = argv[i] + 8;
                }
                else if(strcmp(argv[i] + 2, "case") == 0)
                {
                    g_case_sensitive = 1;
                }
                else if(strcmp(argv[i] + 2, "out") == 0)
                {
                    g_filter_stdout = 1;
                }
            }
            else
            {
                len = strlen(argv[i]);
                for(j = 1; j < len; ++j)
                {
                    switch(argv[i][j])
                    {
                    case 'c':
                        g_case_sensitive = 1;
                        break;

                    case 'h':
                        usage();
                        /*NOTREACHED*/
                        break;

                    case 'r':
                        ++i;
                        if(i >= argc)
                        {
                            std::cerr << g_progname
                                      << ":error: --regex must be followed by a regular expression."
                                      << std::endl;
                            exit(1);
                        }
                        g_regex = argv[i];
                        break;

                    case 'V':
                        std::cout << g_version << std::endl;
                        exit(0);
                        /*NOTREACHED*/
                        break;

                    }
                }
            }
        }
        else
        {
            /* i points to the command we want to run now */
            break;
        }
    }
    if(i >= argc)
    {
        std::cerr << g_progname
                  << ":error: no command specified."
                  << std::endl;
        exit(1);
    }

    /* the next parameter (starting at 'i') is the command and the following
     * ones are its parameters
     */


    /* we want to redirect the child I/O to ourselves so we create a couple
     * of pipes to replace stdout and stderr
     */
    if(pipe2(pipe_out, O_NONBLOCK) != 0)
    {
        std::cerr << g_progname
                  << ":error: could not create pipe to replace stdout."
                  << std::endl;
        exit(1);
    }
    if(pipe2(pipe_err, O_NONBLOCK) != 0)
    {
        std::cerr << g_progname
                  << ":error: could not create pipe to replace stderr."
                  << std::endl;
        exit(1);
    }

    child_pid = fork();
    if(child_pid <= 0)
    {
        int fork_errno(errno);
        close(pipe_out[1]);
        close(pipe_err[1]);

        if(child_pid == 0)
        {
            /* we are the parent and we got a child, duplicate its output
             * except for lines that match the regex
             */
            regcomp(&regex, g_regex, REG_EXTENDED | (g_case_sensitive == 0 ? REG_ICASE : 0) | REG_NOSUB);
            while(pipe_out[0] != -1 || pipe_err[0] != -1)
            {
                memset(&fds, 0, sizeof(fds));
                fds[0].fd = pipe_out[0];
                fds[0].events = POLLIN | POLLPRI | POLLRDHUP;
                fds[0].revents = 0;
                fds[1].fd = pipe_err[0];
                fds[1].events = POLLIN | POLLPRI | POLLRDHUP;
                fds[1].revents = 0;
                count = pipe_out[0] == -1 || pipe_err[0] == -1 ? 1 : 2;
                if(poll(fds + (pipe_out[0] == -1 ? 1 : 0), count, -1) < 0)
                {
                    int const err(errno);
                    std::cerr << g_progname
                              << ":error: poll() returned with -1: "
                              << err
                              << ", "
                              << strerror(err)
                              << "."
                              << std::endl;
                    exit(1);
                }
                if((fds[0].revents & (POLLIN | POLLPRI)) != 0)
                {
                    read_pipe(pipe_out[0], stdout, g_filter_stdout == 0 ? NULL : &regex, &g_buf_out);
                }
                if((fds[1].revents & (POLLIN | POLLPRI)) != 0)
                {
                    read_pipe(pipe_err[0], stderr, &regex, &g_buf_err);
                }
                if((fds[0].revents & (POLLHUP | POLLRDHUP)) != 0)
                {
                    close(pipe_out[0]);
                    pipe_out[0] = -1;
                }
                if((fds[1].revents & (POLLHUP | POLLRDHUP)) != 0)
                {
                    close(pipe_err[0]);
                    pipe_err[0] = -1;
                }
            }
            exit(0);
        }
        else
        {
            std::cerr << g_progname
                      << ":error: fork() failed: "
                      << fork_errno
                      << ", "
                      << strerror(fork_errno)
                      << "."
                      << std::endl;
            exit(1);
        }
    }

    /* here we are the child */

    /* child does not need the readable side of the pipes */
    close(pipe_out[0]);
    close(pipe_err[0]);

    /* redirect stdout/stderr to corresponding pipe */
    dup2(pipe_out[1], 0);
    dup2(pipe_err[1], 2);

    if(strchr(argv[i], '/') == nullptr)
    {
        /* the command will often be written as is, without a path
         * so here we first check whether we can find the command
         *
         * also, not prepending one of the $PATH paths could be a security
         * problem since we'd end up using "./<command>" which is not
         * valid by default...
         */
        path = getenv("PATH");
        if(path == nullptr)
        {
            path = strdup("/usr/bin");
        }
        else
        {
            /* make a copy so that way we can change the ':' in '\0' */
            path = strdup(path);
        }

        cmd_len = strlen(argv[i]);
        for(p = path; *p != '\0'; )
        {
            for(e = p; *e != '\0' && *e != ':'; ++e);
            n = *e == ':' ? e + 1 : e;
            *e = '\0';
            len = strlen(p) + 1 + cmd_len + 1; // path + '/' + command + '\0'
            e = new char[len];
            snprintf(e, len, "%s/%s", p, argv[i]);  // we use snprintf() because 'e' is saved in argv[i] and then we call execve()
            if(access(e, F_OK) == 0)
            {
                if(access(e, R_OK | X_OK) == 0)
                {
                    /* we found the one we want */
                    argv[i] = e;
                    break;
                }
                std::cerr << g_progname
                          << ":error: "
                          << e
                          << " is not an executable."
                          << std::endl;
                exit(1);
            }
            delete [] e;
            p = n;
        }
    }

    /* start command */
    execve(argv[i], argv + i, envp);
    saved_errno = errno;

    /* we reach here if execve() cannot start 'command' */
    std::cerr << g_progname
              << ":error: execve() failed: "
              << strerror(saved_errno)
              << "."
              << std::endl
              << g_progname
              << ":error: Command: "
              << argv[i];
    for(++i; i < argc; ++i)
    {
        std::cerr << " " << argv[i];
    }
    std::cerr << std::endl;

    exit(1);
}

// vim: ts=4 sw=4 et
