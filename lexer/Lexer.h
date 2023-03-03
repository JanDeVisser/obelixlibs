/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <set>

#include <lexer/Tokenizer.h>

namespace Obelix {

class Lexer {
public:
    explicit Lexer(char const* = nullptr, std::string = {});
    explicit Lexer(StringBuffer&, std::string = {});

    template<typename... Args>
    void filter_codes(TokenCode code, Args&&... args)
    {
        m_filtered_codes.insert(code);
        filter_codes(std::forward<Args>(args)...);
    }

    void filter_codes()
    {
    }

    void assign(char const* buffer, std::string file_name={}, bool take_ownership=false);
    void assign(std::string buffer, std::string = {});
    void assign(StringBuffer&& buffer, std::string = {});
    void assign(std::shared_ptr<StringBuffer> buffer, std::string = {});
    [[nodiscard]] std::shared_ptr<StringBuffer> const& buffer() const;
    std::vector<Token> const& tokenize(char const* buffer=nullptr, std::string file_name={}, bool take_ownership=false);
    [[nodiscard]] std::vector<Token> const& tokens() const;
    void invalidate();
    void rewind();
    Token const& peek(size_t = 0);
    Token const& lex();
    Token const& replace(Token const&);
    std::optional<Token const> match(TokenCode);
    TokenCode current_code();
    bool expect(TokenCode);

    template<class ScannerClass, class... Args>
    std::shared_ptr<ScannerClass> add_scanner(Args&&... args)
    {
        auto ret = std::make_shared<ScannerClass>(std::forward<Args>(args)...);
        m_scanners.insert(std::dynamic_pointer_cast<Scanner>(ret));
        return ret;
    }

    std::shared_ptr<Scanner> add_scanner(std::string, CustomScanner::Match, int = 10);

    void mark();
    void discard_mark();
    void rewind_to_mark();

private:
    std::string m_file_name;
    std::shared_ptr<StringBuffer> m_buffer;
    std::vector<Token> m_tokens {};
    size_t m_current { 0 };
    std::vector<size_t> m_bookmarks {};
    std::unordered_set<TokenCode> m_filtered_codes {};
    std::set<std::shared_ptr<Scanner>> m_scanners {};
};

}
