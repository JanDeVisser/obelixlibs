/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/Lexer.h>

namespace Obelix {

logging_category(lexer);

Lexer::Lexer(char const* text, std::string file_name)
    : m_file_name(std::move(file_name))
    , m_buffer(new StringBuffer(text))
{
}

Lexer::Lexer(StringBuffer& text, std::string file_name)
    : m_file_name(std::move(file_name))
    , m_buffer(new StringBuffer(text))
{
}

std::shared_ptr<Scanner> Lexer::add_scanner(std::string name, CustomScanner::Match match, int priority)
{
    auto scanner = std::make_shared<CustomScanner>(std::move(name), std::move(match), priority);
    m_scanners.insert(std::dynamic_pointer_cast<Scanner>(scanner));
    return scanner;
}


void Lexer::assign(char const* text, std::string file_name, bool take_ownership)
{
    m_file_name = std::move(file_name);
    m_buffer->assign(text, take_ownership);
    invalidate();
}

void Lexer::assign(std::string text, std::string file_name)
{
    m_file_name = std::move(file_name);
    m_buffer->assign(std::move(text));
    invalidate();
}

void Lexer::assign(StringBuffer&& buffer, std::string file_name)
{
    m_file_name = std::move(file_name);
    m_buffer = std::make_shared<StringBuffer>(buffer);
}

void Lexer::assign(std::shared_ptr<StringBuffer> buffer, std::string file_name)
{
    m_file_name = std::move(file_name);
    m_buffer = std::move(buffer);
}

void Lexer::assign(std::string_view buffer, std::string file_name)
{
    m_file_name = std::move(file_name);
    m_buffer = std::make_shared<StringBuffer>(buffer);
}

std::shared_ptr<StringBuffer> const& Lexer::buffer() const
{
    return m_buffer;
}

std::vector<Token> const& Lexer::tokenize(char const* text, std::string file_name, bool take_ownership)
{
    if (text != nullptr)
        assign(text, std::move(file_name), take_ownership);
    Tokenizer tokenizer(*m_buffer, m_file_name);
    tokenizer.add_scanners(m_scanners);
    tokenizer.filter_codes(m_filtered_codes);
    tokenizer.tokenize(m_tokens);
    return m_tokens;
}

std::vector<Token> const& Lexer::tokens() const
{
    return m_tokens;
}

void Lexer::invalidate()
{
    m_tokens.clear();
    m_current = 0;
}

void Lexer::rewind()
{
    m_current = 0;
}

Token const& Lexer::peek(size_t how_many)
{
    if (m_tokens.empty())
        tokenize();
    oassert(m_current + how_many < m_tokens.size(), "Token buffer underflow");
    return m_tokens[m_current + how_many];
}

Token const& Lexer::lex()
{
    auto const& ret = peek(0);
    if (m_current < (m_tokens.size() - 1))
        m_current++;
    return ret;
}

Token const& Lexer::replace(Token const& token)
{
    auto const& ret = peek(0);
    m_tokens[m_current] = token;
    return ret;
}

std::optional<Token const> Lexer::match(TokenCode code)
{
    if (peek().code() != code)
        return {};
    return lex();
}

TokenCode Lexer::current_code()
{
    return peek().code();
}

bool Lexer::expect(TokenCode code)
{
    auto token_maybe = match(code);
    return token_maybe.has_value();
}

void Lexer::mark()
{
    m_bookmarks.push_back(m_current);
}

void Lexer::discard_mark()
{
    m_bookmarks.pop_back();
}

void Lexer::rewind_to_mark()
{
    m_current = m_bookmarks.back();
    discard_mark();
}

}
