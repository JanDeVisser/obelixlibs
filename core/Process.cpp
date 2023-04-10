/*
 * Copyright (c) ${YEAR}, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstring>
#include <iostream>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <core/ScopeGuard.h>
#include <core/Process.h>

namespace Obelix {

constexpr int PipeEndRead = 0;
constexpr int PipeEndWrite = 1;

std::mutex Process::s_debug_lock;

Pipe::~Pipe()
{
    close();
}

ErrorOr<void,SystemError> Pipe::create()
{
    if (pipe(m_pipe) == -1) {
        return SystemError { ErrorCode::IOError, "pipe() failed" };
    }
    return {};
}

void Pipe::close()
{
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
}

ReadPipe::~ReadPipe()
{
}

void ReadPipe::connect_parent()
{
    m_fd = m_pipe[PipeEndRead];
    ::close(m_pipe[PipeEndWrite]);
    fcntl(m_fd, F_SETFL, O_NONBLOCK);
    std::thread thread { &ReadPipe::read, this };
    thread.detach();
}

void ReadPipe::connect_child(int fd)
{
    while ((dup2(m_pipe[PipeEndWrite], fd) == -1) && (errno == EINTR)) { }
    ::close(m_pipe[PipeEndRead]);
    ::close(m_pipe[PipeEndWrite]);
}

void ReadPipe::close()
{
    if (!m_current.empty())
        newline();
    m_condition.notify_all();
    Pipe::close();
}

void ReadPipe::read()
{
    struct pollfd poll_fd {0};
    poll_fd.fd = fd();
    poll_fd.events = POLLIN;

    while (true) {
        if (poll(&poll_fd, 1, 0) == -1) {
            if (errno == EINTR) {
                std::cerr << "poll(stdin, stdout) interrupted: " << strerror(errno) << "\n";
                continue;
            }
            std::cerr << "poll(stdin, stdout) failed: " << strerror(errno) << "\n";
            break;
        }
        if (poll_fd.revents & POLLIN)
            drain();
    }
    close();
};

void ReadPipe::drain()
{
    uint8_t buffer[4096];
    std::lock_guard<std::mutex> lock(m_lock);
    for (auto count = ::read(m_fd, buffer, sizeof(buffer) - 1); count != 0; count = ::read(m_fd, buffer, sizeof(buffer) - 1)) {
        if (count > 0) {
            bool prev_was_cr { false };
            for (auto ix = 0u; ix < count; ++ix) {
                auto ch = static_cast<char>(buffer[ix]);
                switch (ch) {
                case '\r':
                    newline();
                    prev_was_cr = true;
                    break;
                case '\n':
                    if (!prev_was_cr) {
                        newline();
                    }
                    prev_was_cr = true;
                    break;
                default:
                    m_current += ch;
                    prev_was_cr = false;
                }
            }
            continue;
        }
        switch (errno) {
        case EINTR:
            continue;
        case EBADF:
        case EAGAIN:
            goto exit_loop;
        default:
            std::cerr << SystemError { ErrorCode::IOError, "Error reading child process output" }.to_string() << "\n";
            goto exit_loop;
        }
    }
exit_loop:
    if (!m_current.empty())
        newline();
    m_condition.notify_all();
}

void ReadPipe::newline()
{
    Process::log("<- {} | {}", m_name, m_current);
    m_lines.emplace_back(std::move(m_current));
    m_current.clear();
}

std::vector<std::string> ReadPipe::lines()
{
    std::lock_guard<std::mutex> const lock(m_lock);
    std::vector<std::string> ret;
    while (!m_lines.empty()) {
        auto l = m_lines.front();
        ret.push_back(std::move(l));
        m_lines.pop_front();
    }
    return ret;
}

void ReadPipe::expect()
{
    std::unique_lock lock(m_lock);
    while (true) {
        if (!m_lines.empty())
            return;
        m_condition.wait(lock);
    }
}

void WritePipe::connect_parent()
{
    m_fd = m_pipe[PipeEndWrite];
    ::close(m_pipe[PipeEndRead]);
}

void WritePipe::connect_child(int fd)
{
    while ((dup2(m_pipe[PipeEndRead], fd) == -1) && (errno == EINTR)) { }
    ::close(m_pipe[PipeEndRead]);
    ::close(m_pipe[PipeEndWrite]);
}

ErrorOr<int, SystemError> WritePipe::write(std::string const& str)
{
    return write((uint8_t *) str.c_str(), str.length());
}

ErrorOr<int, SystemError> WritePipe::write(std::string_view const& str)
{
    return write((uint8_t *) str.data(), str.length());
}

ErrorOr<int, SystemError> WritePipe::write(uint8_t const* buffer, size_t bytes)
{
    ssize_t count;
    while (true) {
        count = ::write(m_fd, buffer, bytes);
        if (count >= 0)
            break;
        if (errno != EINTR)
            return SystemError { ErrorCode::IOError, "Error writing to child process input" };
    }
    return count;
}

ErrorOr<int, SystemError> execute(std::string const& cmd, std::vector<std::string> const& args)
{
    Process p(cmd, args);
    return p.execute();
}

Process::Process(std::string command, std::vector<std::string> arguments)
    : m_command(std::move(command))
    , m_arguments(std::move(arguments))
{
}

std::vector<std::string> Process::standard_out()
{
    return m_stdout.lines();
}

std::vector<std::string> Process::standard_error()
{
    return m_stderr.lines();
}

ErrorOr<int, SystemError> Process::execute()
{
    TRY_RETURN(start());
    return this->wait();
}

ErrorOr<void, SystemError> Process::background()
{
    TRY_RETURN(start());
    return {};
}

ErrorOr<int, SystemError> Process::wait()
{
    if (m_pid == 0)
        return 0;
    int exit_code;
    if (waitpid(m_pid, &exit_code, 0) == -1 && errno != ECHILD && errno != EINTR)
        return SystemError { ErrorCode::IOError, "waitpid() failed" };
    m_pid = 0;
    m_stdin.close();
    if (!WIFEXITED(exit_code))
        return SystemError { ErrorCode::IOError, "Child program {} crashed due to signal {}", m_command, WTERMSIG(exit_code) };
    return WEXITSTATUS(exit_code);
}

ErrorOr<void, SystemError> Process::terminate()
{
    m_stdin.close();
    if (auto err = wait(); err.is_error())
        return err.error();
    return {};
}

ErrorOr<void, SystemError> Process::start()
{
    auto sz = m_arguments.size();
    char** argv = new char*[sz + 2];
    argv[0] = new char[m_command.length() + 1];
    strcpy(argv[0], m_command.c_str());
    for (auto ix = 0u; ix < sz; ++ix) {
        argv[ix + 1] = new char[m_arguments[ix].length() + 1];
        strcpy(argv[ix + 1], m_arguments[ix].c_str());
    }
    argv[sz + 1] = nullptr;
    log(format("***  | {} {}", m_command, join(m_arguments, ' ')));

    ScopeGuard sg([&argv, &sz]() {
        for (auto ix = 0u; ix < sz; ++ix)
            delete[] argv[ix];
        delete[] argv;
    });

    signal(SIGCHLD, SIG_IGN);

    TRY_RETURN(m_stdin.create());
    TRY_RETURN(m_stdout.create());
    TRY_RETURN(m_stderr.create());

    m_pid = fork();
    if (m_pid == -1)
        return SystemError { ErrorCode::IOError, "fork() failed" };
    if (m_pid == 0) {
        m_stdin.connect_child(STDIN_FILENO);
        m_stdout.connect_child(STDOUT_FILENO);
        m_stderr.connect_child(STDERR_FILENO);
        execvp(m_command.c_str(), argv);
        return SystemError { ErrorCode::IOError, "execvp() failed" };
    }
    m_stdin.connect_parent();
    m_stdout.connect_parent();
    m_stderr.connect_parent();
    return {};
}

bool Process::running() const
{
    return m_pid != 0;
}

ErrorOr<int, SystemError> Process::write(std::string const& str)
{
    log("-> I | {}", str);
    return m_stdin.write(str);
}

ErrorOr<int, SystemError> Process::write(std::string_view const& str)
{
    return m_stdin.write(str);
}

ErrorOr<int, SystemError> Process::write(uint8_t const* buffer, size_t bytes)
{
    return m_stdin.write(buffer, bytes);
}


}
