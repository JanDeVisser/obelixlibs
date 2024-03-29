/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/Token.h>

namespace Obelix {

std::string TokenCode_name(TokenCode t)
{
    switch (t) {
#undef ENUM_TOKEN_CODE
#define ENUM_TOKEN_CODE(code, c, str) \
    case TokenCode::code:             \
        if (str != nullptr)           \
            return str;               \
        return #code;
        ENUMERATE_TOKEN_CODES(ENUM_TOKEN_CODE)
#undef ENUM_TOKEN_CODE
    default:
        return format("Custom ({})", (int)t);
    }
}

bool Location::operator==(Location const& other) const
{
    return index == other.index;
}

bool Location::operator<(Location const& other) const
{
    return index < other.index;
}

std::string Location::to_string() const
{
    return format("{}:{}", line, column);
}

std::set<std::string> Span::s_files;

Span::Span(std::string_view fname, Location loc_1, Location loc_2)
    : file_name(fname)
    , start(loc_1)
    , end(loc_2)
{
}

Span::Span(std::string_view fname, size_t line_1, size_t col_1, size_t line_2, size_t col_2)
    : file_name(fname)
    , start({ line_1, col_1 })
    , end({ line_2, col_2 })
{
}

Span::Span(char const* fname, Location loc_1, Location loc_2)
    : file_name(fname)
    , start(loc_1)
    , end(loc_2)
{
}

Span::Span(char const* fname, size_t line_1, size_t col_1, size_t line_2, size_t col_2)
    : file_name(fname)
    , start({ line_1, col_1 })
    , end({ line_2, col_2 })
{
}

Span::Span(std::string const& fname, Location loc_1, Location loc_2)
    : start(loc_1)
    , end(loc_2)
{
    if (!s_files.contains(fname)) {
        s_files.emplace(fname);
    }
    auto it = s_files.find(fname);
    assert(it != s_files.end());
    file_name = *it;
}

Span::Span(std::string const& fname, size_t line_1, size_t col_1, size_t line_2, size_t col_2)
    : start({ line_1, col_1 })
    , end({ line_2, col_2 })
{
    if (!s_files.contains(fname)) {
        s_files.emplace(fname);
    }
    auto it = s_files.find(fname);
    assert(it != s_files.end());
    file_name = *it;
}

std::string Span::to_string() const
{
    if (!empty()) {
        return (!file_name.empty())
            ? format("{}:{}-{}", file_name, start, end)
            : format("{}-{}", start, end);
    } else {
        return format("{}:", file_name);
    }
}

[[nodiscard]] bool Span::empty() const
{
    return start == end;
}

bool Span::operator==(Span const& other) const
{
    return file_name == other.file_name && start == other.start && end == other.end;
}

Span Span::merge(Span const& other) const
{
    size_t new_start_line = start.line;
    if (other.start.line < new_start_line)
        new_start_line = other.start.line;
    size_t new_end_line = end.line;
    if (other.end.line > new_end_line)
        new_end_line = other.end.line;
    size_t new_start_column = start.column;
    if (other.start.column < new_start_column)
        new_start_column = other.start.column;
    size_t new_end_column = end.column;
    if (other.end.column > new_end_column)
        new_end_column = other.end.column;
    return Span { file_name, new_start_line, new_start_column, new_end_line, new_end_column };
}

std::string Token::to_string() const
{
    std::string ret = code_name();
    if (!m_value.empty()) {
        ret += format(" [{}]", value());
    }
    return ret;
}

std::optional<long> Token::to_long() const
{
    return Obelix::to_long(m_value);
}

std::optional<double> Token::to_double() const
{
    return Obelix::to_double(m_value);
}

std::optional<bool> Token::to_bool() const
{
    auto number_maybe = to_long();
    if (number_maybe.has_value())
        return number_maybe.value() != 0;
    return Obelix::to_bool(m_value);
}

int Token::compare(Token const& other) const
{
    if (m_code == other.m_code)
        return m_value.compare(other.m_value);
    else
        return (int)m_code - (int)other.m_code;
}

bool Token::is_whitespace() const
{
    return (code() == TokenCode::Whitespace) || (code() == TokenCode::NewLine);
}
}
