/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/Tokenizer.h>

namespace Obelix {

extern_logging_category(lexer);

class CatchAll : public Scanner {
public:
    explicit CatchAll()
        : Scanner(99)
    {
    }

    void match(Tokenizer& tokenizer) override
    {
    }

    [[nodiscard]] char const* name() const override { return "catchall"; }

private:
};

Tokenizer::Tokenizer(std::string_view const& text, std::string file_name)
    : m_buffer(text)
    , m_file_name(std::move(file_name))
{
}

Tokenizer::Tokenizer(StringBuffer text, std::string file_name)
    : m_buffer(std::move(text))
    , m_file_name(std::move(file_name))
{
}

std::vector<Token> const& Tokenizer::tokenize(std::vector<Token>& tokens)
{
    debug(lexer, "Scanners:");
    for (auto &scanner : m_scanners) {
        debug(lexer, "{} priority {}", scanner->name(), scanner->priority());
    }
    m_tokens = &tokens;
    while (tokens.empty() || tokens.back().code() != TokenCode::EndOfFile) {
        match_token();
    }
    oassert(!tokens.empty(), "tokenize() found no tokens, not even EOF");
    oassert(tokens.back().code() == TokenCode::EndOfFile, "tokenize() did not leave an EOF");
    return tokens;
}

void Tokenizer::match_token()
{
    debug(lexer, "tokenizer::match_token");
    m_state = TokenizerState::Init;

    if (m_locked_scanner != nullptr) {
        m_current_scanner = m_locked_scanner;
        auto name = m_locked_scanner->name();
        debug(lexer, "Matching with locked scanner '{}'", name);
        rewind();
        m_locked_scanner->match(*this);
        oassert(m_state == TokenizerState::Success, "Match with locked scanner {} failed", name);
    } else {
        for (auto &scanner: m_scanners) {
            debug(lexer, "Matching with scanner '{}'", scanner->name());
            m_current_scanner = scanner;
            rewind();
            scanner->match(*this);
            if (m_state == TokenizerState::Success) {
                debug(lexer, "Match with scanner {} succeeded", scanner->name());
                break;
            }
        }

        if (state() != TokenizerState::Success) {
            rewind();
            debug(lexer, "Catchall scanner");
            auto ch = peek();
            if (ch > 0) {
                push();
                accept(TokenCode_by_char(ch));
            }
        }
    }

    if (m_buffer.eof()) {
        debug(lexer, "End-of-file. Accepting TokenCode::EndOfFile");
        accept_token(TokenCode::EndOfFile, "End of File Marker");
    }
}

std::string const& Tokenizer::token() const
{
    return m_token;
}

void Tokenizer::chop(size_t num)
{
    m_token.erase(0, num);
}

TokenizerState Tokenizer::state() const
{
    return m_state;
}

bool Tokenizer::at_top() const
{
    return m_buffer.top();
}

bool Tokenizer::at_end() const
{
    return m_buffer.eof();
}

/**
 * Rewind the tokenizer to the point just after the last token was identified.
 */
void Tokenizer::rewind()
{
    debug(lexer, "Rewinding tokenizer");
    m_token = "";
    m_buffer.rewind();
}

void Tokenizer::partial_rewind(size_t num)
{
    if (num > m_token.length())
        num = m_token.length();
    if (num < m_token.length())
        m_token = m_token.substr(0, m_token.length() - num);
    else
        m_token = "";
    m_buffer.partial_rewind(num);
}

/**
 * Mark the current point, discarding everything that came before it.
 */
void Tokenizer::reset() {
    debug(lexer, "Resetting tokenizer");
    auto scanned = m_buffer.scanned_string();
    for (auto ix = 0u; ix < scanned.length(); ++ix) {
        auto ch = scanned[ix];
        if (ch == '\r' & ix < scanned.length()-1 && scanned[ix+1] == '\n')
            ch = scanned[++ix];
        if (ch == '\n' || ch == '\r') {
            m_mark.line++;
            m_mark.column = 1;
        } else {
            m_mark.column++;
        }
    }
    m_buffer.reset();
    m_token = "";
}

std::string const& Tokenizer::current_token() const
{
    return m_token;
}

Token Tokenizer::accept(TokenCode code)
{
    return accept_token(code, m_token);
}

Token Tokenizer::accept_token(TokenCode code, std::string value)
{
    Token ret = Token(code, std::move(value));
    return accept_token(ret);
}

Token Tokenizer::accept_token(Token& token)
{
    auto mark = m_mark;
    skip();
    // reset(), and therefore skip(), sets the mark to the
    // current point:
    token.location(Span { m_file_name, mark, m_mark });
    debug(lexer, "Lexer::accept_token({})", token.to_string());
    if (!m_filtered_codes.contains(token.code()))
        m_tokens->push_back(token);
    return token;
}

void Tokenizer::skip()
{
    reset();
    m_state = TokenizerState::Success;
}

void Tokenizer::push() {
    push_as(m_current);
}

void Tokenizer::push_as(int ch) {
    m_buffer.readchar();
    if (ch)
        m_token += (char) ch;
    m_current = 0;
}

void Tokenizer::discard() {
    push_as(0);
}

int Tokenizer::peek(int num)
{
    auto ret = m_buffer.peek(num);
    if (num == 0)
        m_current = ret;
    debug(lexer, "peek() = {}", ret);
    return ret;
}

void Tokenizer::add_scanners(std::set<std::shared_ptr<Scanner>> scanners)
{
    m_scanners.merge(scanners);
}

std::shared_ptr<Scanner> Tokenizer::get_scanner(std::string const& name)
{
    for (auto& scanner : m_scanners) {
        if (scanner->name() == name) {
            return scanner;
        }
    }
    return nullptr;
}

void Tokenizer::lock_scanner()
{
    m_locked_scanner = m_current_scanner;
}

void Tokenizer::unlock_scanner()
{
    m_locked_scanner = nullptr;
}

}
