/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

//
// Created by Jan de Visser on 2021-09-20.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Obelix {

int stricmp(const char*, const char*);
std::string to_upper(std::string const&);
std::string to_lower(std::string const&);
std::size_t replace_all(std::string&, std::string_view, std::string_view);

std::string c_escape(std::string const& s);
std::vector<std::string> split(std::string const& s, char sep);
std::string join(std::vector<std::string> const& collection, std::string const& sep);
std::string join(std::vector<std::string> const& collection, char sep);
std::string strip(std::string const& s);
std::string rstrip(std::string const& s);
std::string lstrip(std::string const& s);
std::vector<std::pair<std::string, std::string>> parse_pairs(std::string const& s, char pair_sep = ';', char name_value_sep = '=');

template<typename ElementType, typename ToString>
inline std::string join(std::vector<ElementType> const& collection, std::string const& sep, ToString const& tostring)
{
    std::string ret;
    auto first = true;
    for (auto& elem : collection) {
        if (!first) {
            ret += sep;
        }
        ret += tostring(elem);
        first = false;
    }
    return ret;
}

template<typename ElementType, typename ToString>
inline std::string join(std::vector<ElementType> const& collection, char sep, ToString const& tostring)
{
    std::string sep_str;
    sep_str += sep;
    return join(collection, sep_str, tostring);
}

template <std::integral Int>
[[nodiscard]] inline std::string integer_to_string(Int integer, int radix = 10, char grouping_char = '\0')
{
    thread_local static char buf[128];
    char* ptr = &buf[127];
    *ptr = 0;

    int ix = 0;

    typedef typename std::make_unsigned<Int>::type UInt;
    UInt integer_pos = integer;
    if (std::is_signed_v<Int> && (integer < 0))
        integer_pos = -integer;

    do {
        if (grouping_char && (++ix % 3) == 0)
            *(--ptr) = grouping_char;
        *(--ptr) = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[integer_pos % radix];
        integer_pos /= radix;
    } while (integer_pos > 0);
    if (std::is_signed_v<Int> && (integer < 0)) {
        *(--ptr) = '-';
    }
    return { ptr };
}

template <std::integral Int, typename Str=std::string>
[[nodiscard]] inline std::optional<Int> string_to_integer(Str const& s, int radix = 0)
{
    int start { 0 };
    for (start = 0; isspace(s[start]) && start < s.length(); ++start);
    auto ch = s[start];
    switch (ch) {
    case '0': {
        if (start < s.length() - 2) {
            switch (toupper(s[start + 1])) {
            case 'X': {
                if (radix != 0 && radix != 16)
                    return {};
                radix = 16;
                start += 2;
            } break;
            case 'B': {
                if (radix != 0 && radix != 16)
                    return {};
                radix = 16;
                start += 2;
            } break;
            default:
                break;
            }
        }
    } break;
    case '$': {
        if (radix != 0 && radix != 16)
            return {};
        radix = 16;
        ++start;
    } break;
    default:
        break;
    }
    if (radix == 0)
        radix = 10;
    assert(radix < 36);

    Int val { 0 };
    uint64_t exp { 1 };
    auto trailing_ws = true;
    for (int ix = s.length()-1; ix >= start; --ix) {
        ch = s[ix];
        if (trailing_ws && isspace(ch))
            continue;
        trailing_ws = false;
        if (std::is_signed_v<Int> && ch == '-') {
            if ((ix > start) || (exp == 1))
                return {};
            val = -val;
            continue;
        }
        char digit =  (isdigit(ch)) ? (ch - '0') : ((toupper(ch) - 'A') + 10);
        if (digit < 0 || digit >= radix)
            return {};
        val += digit * exp;
        exp *= radix;
    }
    return val;
}

// -- to_string -------------------------------------------------------------

template<typename T>
inline std::string to_string(T value)
{
    return value.to_string();
}

template<>
inline std::string to_string(std::string const& value)
{
    return value;
}

template<>
inline std::string to_string(std::string value)
{
    return value;
}

template<>
inline std::string to_string(std::string* value)
{
    return *value;
}

template<>
inline std::string to_string(std::string_view const& value)
{
    return static_cast<std::string>(value);
}

template<>
inline std::string to_string(std::string_view value)
{
    return static_cast<std::string>(value);
}

template<>
inline std::string to_string(char* value)
{
    return { value };
}

template<>
inline std::string to_string(char const* value)
{
    return { value };
}

template<int N>
inline std::string to_string(char value[N])
{
    return { value };
}

template<int N>
inline std::string to_string(char const value[N])
{
    return { value };
}

template <std::integral T>
inline std::string to_string(T value)
{
    return integer_to_string(value);
}

template<std::unsigned_integral T>
std::string to_hex_string(T value)
{
    return integer_to_string(value, 16);
}

template <>
inline std::string to_string(void* val)
{
    return integer_to_string(reinterpret_cast<uintptr_t>(val));
}

template<std::floating_point Float>
inline std::string to_string(Float value)
{
    return std::to_string(value);
}

template<>
inline std::string to_string(bool const& value)
{
    return (value) ? "true" : "false";
}

template <typename T>
inline std::string to_string(std::shared_ptr<T> value)
{
    if (value == nullptr)
        return "(null)";
    return to_string<T>(*value);
}

template <typename T>
inline std::string to_string(std::unique_ptr<T> value)
{
    if (value == nullptr)
        return "(null)";
    return to_string<T>(*value);
}

template <typename T>
inline std::string to_string(std::vector<T> const& value)
{
    return join(value, ", ", [](T elem) { return elem.to_string(); });
}

template <typename T>
inline std::string to_string(std::vector<T*> const& value)
{
    return join(value, ", ", [](T* elem) { return elem->to_string(); });
}

template <typename T>
inline std::string to_string(std::vector<std::shared_ptr<T>> const& value)
{
    return join(value, ", ", [](std::shared_ptr<T> elem) { return elem->to_string(); });
}

template <typename T>
inline std::string to_string(std::vector<std::unique_ptr<T>> const& value)
{
    return join(value, ", ", [](std::unique_ptr<T> elem) { return elem->to_string(); });
}

// -- to_long ---------------------------------------------------------------

template <typename T>
inline std::optional<long> try_to_long(T str)
{
    return string_to_integer<long>(to_string(str));
}

/*
template <>
inline std::optional<long> try_to_long(std::string str)
{
    return string_to_integer<long>(str);
}
 */

template <>
inline std::optional<long> try_to_long(std::string const& str)
{
    return string_to_integer<long>(str);
}

template <>
inline std::optional<long> try_to_long(std::string_view const& str)
{
    return string_to_integer<long>(str);
}

template <std::integral Int>
inline std::optional<long> try_to_long(Int val)
{
    return static_cast<long>(val);
}

template <std::floating_point Float>
inline std::optional<long> try_to_long(Float val)
{
    return val;
}

template <>
inline std::optional<long> try_to_long(bool value)
{
    return (value) ? 1 : 0;
}


template <typename T>
std::string try_to_long(std::vector<T> const& value)
{
    return value.size();
}

template <typename T=std::string>
long to_long(T str)
{
    auto ret = try_to_long(str);
    assert(ret.has_value());
    return ret.value();
}

// -- to_ulong --------------------------------------------------------------

template <typename T>
inline std::optional<unsigned long> try_to_ulong(T value)
{
    if (auto signed_long = try_to_long(value); signed_long.has_value())
        return signed_long.value();
    return {};
}

template <>
inline std::optional<unsigned long> try_to_ulong(std::string const& str)
{
    return string_to_integer<unsigned long>(str);
}

template <std::unsigned_integral UInt>
inline std::optional<unsigned long> try_to_ulong(UInt val)
{
    return static_cast<unsigned long>(val);
}

template <std::signed_integral SInt>
inline std::optional<unsigned long> try_to_ulong(SInt val)
{
    if (val < 0)
        return {};
    return static_cast<unsigned long>(val);
}

template <std::floating_point Float>
inline std::optional<unsigned long> try_to_ulong(Float val)
{
    if (val < 0)
        return {};
    return val;
}

template <>
inline std::optional<unsigned long> try_to_ulong(bool value)
{
    return (value) ? 1 : 0;
}

template <typename T>
inline std::optional<unsigned long> try_to_ulong(std::vector<T> const& value)
{
    return value.size();
}

template <typename T=std::string>
inline unsigned long to_ulong(T str)
{
    auto ret = try_to_long(str);
    assert(ret.has_value());
    return ret.value();
}

// -- to_double -------------------------------------------------------------

template <typename T>
inline std::optional<double> try_to_double(T str)
{
    return try_to_double(to_string(str));
}

template <>
inline std::optional<double> try_to_double(std::string str)
{
    char* end;
    errno = 0;
    auto ret = strtod(str.c_str(), &end);
    if ((end != (str.c_str() + str.length())) || (errno != 0))
        return {};
    if (ret == 0) {
        for (auto& ch : str) {
            if ((ch != '.') && (ch != '0'))
                return {};
        }
    }
    return ret;
}

template <std::integral Int>
inline std::optional<double> try_to_double(Int value)
{
    return static_cast<double>(value);
}

template <std::floating_point Float>
inline std::optional<double> try_to_double(Float value)
{
    return value;
}

template <>
inline std::optional<double> try_to_double(bool value)
{
    return (value) ? 1.0 : 0.0;
}

template <typename T=std::string>
inline double to_double(T str)
{
    auto ret = try_to_double(str);
    assert(ret.has_value());
    return ret.value();
}

// -- to_bool ---------------------------------------------------------------

template <typename T>
inline std::optional<bool> try_to_bool(T value)
{
    if (auto l = try_to_long(value); l.has_value())
        return try_to_bool(l.value());
    return {};
}

template <>
inline std::optional<bool> try_to_bool(std::string const& str)
{
    if (stricmp(str.c_str(), "true") == 0)
        return true;
    if (stricmp(str.c_str(), "false") == 0)
        return false;
    auto ret = try_to_long(str);
    if (!ret.has_value())
        return {};
    return (ret.value() != 0);
}

template <typename T=std::string>
inline bool to_bool(T const& str)
{
    auto ret = try_to_bool(str);
    assert(ret.has_value());
    return ret.value();
}

}
