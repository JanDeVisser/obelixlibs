/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cassert>
#include <cctype>
#include <cstring>

#include <core/StringUtil.h>

namespace Obelix {

int stricmp(const char* a, const char* b)
{
    unsigned char ca, cb;
    do {
        ca = tolower(toupper((unsigned char)*a++));
        cb = tolower(toupper((unsigned char)*b++));
    } while (ca == cb && ca != '\0');
    return ca - cb;
}

std::string to_upper(std::string const& input)
{
    std::string ret;
    for (auto& ch : input) {
        ret += (char) toupper((int) ch);
    }
    return ret;
}

std::string to_lower(std::string const& input)
{
    std::string ret;
    for (auto& ch : input) {
        ret += (char) tolower((int) ch);
    }
    return ret;
}

std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with)
{
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

std::string c_escape(std::string const& s)
{
    std::string ret;
    for (char c : s) {
        if (c == '"' || c == '\'' || c == '\\') {
            ret += "\\";
        }
        ret += c;
    }
    return ret;
}

std::vector<std::string> split(std::string const& s, char sep)
{
    auto start = 0u;
    auto ptr = 0u;
    std::vector<std::string> ret;
    do {
        start = ptr;
        for (; (ptr < s.length()) && (s[ptr] != sep); ptr++)
            ;
        ret.push_back(s.substr(start, ptr - start));
        start = ++ptr;
    } while (ptr < s.length());
    if (s[s.length() - 1] == sep) // This is ugly...
        ret.emplace_back("");
    return ret;
}

std::string join(std::vector<std::string> const& collection, std::string const& sep)
{
    return join(collection, sep, [](std::string const& elem) { return elem; });
}

std::string join(std::vector<std::string> const& collection, char sep)
{
    return join(collection, sep, [](std::string const& elem) { return elem; });
}

std::string strip(std::string const& s)
{
    size_t start;
    for (start = 0; (start < s.length()) && std::isspace(s[start]); start++)
        ;
    if (start == s.length()) {
        return {};
    }
    size_t end;
    for (end = s.length() - 1; std::isspace(s[end]); end--)
        ;
    return s.substr(start, end - start + 1);
}

std::string rstrip(std::string const& s)
{
    size_t end;
    for (end = s.length() - 1; std::isspace(s[end]); end--)
        ;
    return s.substr(0, end + 1);
}

std::string lstrip(std::string const& s)
{
    size_t start;
    for (start = 0; (start < s.length()) && std::isspace(s[start]); start++)
        ;
    return s.substr(start, s.length() - start);
}

std::vector<std::pair<std::string, std::string>> parse_pairs(std::string const& s, char pair_sep, char name_value_sep)
{
    auto pairs = split(s, pair_sep);
    std::vector<std::pair<std::string, std::string>> ret;
    for (auto& pair : pairs) {
        auto nvp = split(strip(pair), name_value_sep);
        switch (nvp.size()) {
        case 0:
            break;
        case 1:
            if (!strip(nvp[0]).empty())
                ret.emplace_back(strip(nvp[0]), "");
            break;
        case 2:
            if (!strip(nvp[0]).empty())
                ret.emplace_back(strip(nvp[0]), strip(nvp[1]));
            break;
        default:
            if (!strip(nvp[0]).empty()) {
                std::vector<std::string> tail;
                for (auto ix = 1u; ix < nvp.size(); ix++) {
                    tail.push_back(nvp[ix]);
                }
                auto value = strip(join(tail, name_value_sep));
                ret.emplace_back(strip(nvp[0]), strip(value));
            }
        }
    }
    return ret;
}

std::string dequote(std::string const& str, char quote)
{
    auto s = strip(str);
    if (!s.empty() && s.starts_with(quote) && s.ends_with(quote))
        s = s.substr(1, s.length()-2);
    return s;
}

unsigned long parse_binary(char const* str, char** end)
{
    unsigned long ret = 0;
    if ((strlen(str) > 2) && (str[0] == '0') && (toupper(str[1]) == 'B')) {
        str += 2;
    }
    for (*end = const_cast<char*>(str); **end; (*end)++) {
        if (**end == '0') {
            ret <<= 1;
            continue;
        }
        if (**end == '1') {
            ret = (ret<<1) + 1;
            continue;
        }
        break;
    }
    return ret;
}

}
