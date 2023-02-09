/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdarg>
#include <gtest/gtest.h>
#include <lexer/Lexer.h>
#include <lexer/Tokenizer.h>

class LexerTest : public ::testing::Test {
public:
    Obelix::Lexer lexer {};

protected:
    void SetUp() override {
        if (debugOn()) {
            Obelix::Logger::get_logger().enable("lexer");
        }
        initialize();
    }

    virtual void initialize()
    {
        add_scanner<Obelix::QStringScanner>();
        add_scanner<Obelix::NumberScanner>();
        add_scanner<Obelix::IdentifierScanner>();
        add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, false });
    }

    void tokenize(std::string const& s)
    {
        m_text = s;
        tokenize(m_text.c_str());
    }

    void tokenize(char const* text = nullptr)
    {
        lexer.tokenize(text);
        for (auto& token : lexer.tokens()) {
            auto tokens_for = tokens_by_code[token.code()];
            tokens_for.push_back(token);
            tokens_by_code[token.code()] = tokens_for;
        }
    }

    void check_codes(size_t count, ...)
    {
        EXPECT_EQ(count, lexer.tokens().size());
        va_list codes;
        va_start(codes, count);
        for (auto ix = 0u; (ix < count) && (ix < lexer.tokens().size()); ix++) {
            Obelix::TokenCode code = va_arg(codes, Obelix::TokenCode);
            EXPECT_EQ(lexer.tokens()[ix].code_name(), TokenCode_name(code));
        }
    }

    size_t count_tokens_with_code(Obelix::TokenCode code)
    {
        return tokens_by_code[code].size();
    }

    template<class ScannerClass, class... Args>
    std::shared_ptr<ScannerClass> add_scanner(Args&&... args)
    {
        auto ret = lexer.add_scanner<ScannerClass>(std::forward<Args>(args)...);
        return ret;
    }

    std::unordered_map<Obelix::TokenCode, std::vector<Obelix::Token>> tokens_by_code {};

    void TearDown() override {
    }

    virtual bool debugOn() {
        return false;
    }

    std::string m_text;
};
