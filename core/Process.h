/*
 * Copyright (c) 2022, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <core/Error.h>

namespace Obelix {

ErrorOr<int,SystemError> execute(std::string const&, std::vector<std::string> const& = {});

template<typename... Args>
ErrorOr<int,SystemError> execute(std::string const& name, std::vector<std::string>& cmd_args, std::string const& arg, Args&&... args)
{
    cmd_args.push_back(arg);
    return execute(name, cmd_args, std::forward<Args>(args)...);
}

template<typename... Args>
ErrorOr<int,SystemError> execute(std::string const& name, std::string const& arg, Args&&... args)
{
    std::vector<std::string> cmd_args;
    cmd_args.push_back(arg);
    return execute(name, cmd_args, std::forward<Args>(args)...);
}

class Pipe {
public:
    virtual ~Pipe();
    ErrorOr<void,SystemError> create();
    virtual void connect_parent() = 0;
    virtual void connect_child(int) = 0;
    virtual void close();
    [[nodiscard]] int fd() const { return m_fd; }

protected:
    int m_pipe[2] { -1, -1 };
    int m_fd { -1 };
};

class ReadPipe : public Pipe {
public:
    ReadPipe(std::string name)
        : m_name(std::move(name))
    {
    }

    ~ReadPipe();
    void connect_parent() override;
    void connect_child(int) override;
    void close() override;
    void read();
    std::vector<std::string> lines();
    void expect();

private:
    void drain();
    void newline();

    std::string m_name;
    std::string m_current;
    std::deque<std::string> m_lines;
    std::mutex m_lock;
    std::condition_variable m_condition;
};

class WritePipe : public Pipe {
public:
    void connect_parent() override;
    void connect_child(int) override;
    ErrorOr<int,SystemError> write(std::string const&);
    ErrorOr<int,SystemError> write(std::string_view const&);
    ErrorOr<int,SystemError> write(uint8_t const*, size_t);
};

class Process {
public:
    explicit Process(std::string, std::vector<std::string>);

    template<typename... Args>
    explicit Process(std::string const& name, Args&&... args)
        : Process(name, std::vector<std::string> {})
    {
        add_arguments(std::forward<Args>(args)...);
    }

    ErrorOr<int,SystemError> execute();
    ErrorOr<void,SystemError> background();
    ErrorOr<void,SystemError> terminate();
    ErrorOr<int,SystemError> wait();
    bool running() const;
    [[nodiscard]] WritePipe& in() { return m_stdin; }
    [[nodiscard]] ReadPipe& out() { return m_stdout; }
    [[nodiscard]] ReadPipe& err() { return m_stderr; }
    [[nodiscard]] std::vector<std::string> standard_out();
    [[nodiscard]] std::vector<std::string> standard_error();
    ErrorOr<int,SystemError> write(std::string const&);
    ErrorOr<int,SystemError> write(std::string_view const&);
    ErrorOr<int,SystemError> write(uint8_t const*, size_t);

    template <typename... Args>
    static void log(std::string const& fmt, Args&& ...args)
    {
        std::unique_lock lock(s_debug_lock);
        std::string msg;
        if constexpr (sizeof...(Args) > 0) {
            msg = format(fmt, std::forward<Args&&>(args)...);
        } else {
            msg = fmt;
        }
        std::cout << msg << "\n";
    }

private:
    ErrorOr<void,SystemError> start();

    template<typename... Args>
    void add_arguments(std::string const& argument, Args&&... args)
    {
        m_arguments.push_back(argument);
        add_arguments(std::forward<Args>(args)...);
    }

    void add_arguments()
    {
    }

    std::string m_command;
    std::vector<std::string> m_arguments = {};
    pid_t m_pid { 0 };
    WritePipe m_stdin;
    ReadPipe m_stdout { "O" };
    ReadPipe m_stderr { "E" };
    std::mutex m_lock;
    static std::mutex s_debug_lock;
};

}
