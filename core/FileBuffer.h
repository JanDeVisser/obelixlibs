/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <filesystem>
#include <memory>

#include <core/Error.h>
#include <core/ScopeGuard.h>
#include <core/StringBuffer.h>

namespace Obelix {

namespace fs=std::filesystem;

template <>
inline std::string to_string(fs::path value)
{
    return value.string();
}

class BufferLocator {
public:
    virtual ~BufferLocator() = default;
    [[nodiscard]] virtual ErrorOr<fs::path, SystemError> locate(std::string const&) const = 0;

protected:
    [[nodiscard]] static ErrorOr<void, SystemError> check_existence(fs::path const& file_name);
};

class SimpleBufferLocator : public BufferLocator {
public:
    SimpleBufferLocator() = default;
    [[nodiscard]] ErrorOr<fs::path, SystemError> locate(std::string const&) const override;
};


class FileBuffer : public StringBuffer {
public:
    FileBuffer(fs::path, char const*, bool = false);
    static ErrorOr<FileBuffer*, SystemError> from_file(std::string const&, BufferLocator* = nullptr);

    fs::path const& file_path() const { return m_path; }
private:
    fs::path m_path;
};

}
