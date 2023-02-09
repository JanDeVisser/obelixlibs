/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <lexer/Tokenizer.h>
#include <lexer/test/LexerTest.h>

TEST(TokenizerTest, tokenizer_create)
{
    Obelix::Tokenizer tokenizer("1 + 2 + a");
    tokenizer.add_scanner<Obelix::NumberScanner>();
    tokenizer.add_scanner<Obelix::IdentifierScanner>();
    tokenizer.add_scanner<Obelix::WhitespaceScanner>();
    EXPECT_EQ(tokenizer.state(), Obelix::TokenizerState::Fresh);
}

class SimpleLexerTest : public LexerTest {
protected:

    void initialize() override
    {
        add_scanner<Obelix::IdentifierScanner>();
        add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, false });
    }

    bool debugOn() override {
        return false;
    }
};

TEST_F(SimpleLexerTest, SimplestTest)
{
    tokenize("A");
    check_codes(2,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[0].value(), "A");
    EXPECT_EQ(lexer.tokens()[0].location().start.line, 1);
    EXPECT_EQ(lexer.tokens()[0].location().start.column, 1);
    EXPECT_EQ(lexer.tokens()[0].location().end.line, 1);
    EXPECT_EQ(lexer.tokens()[0].location().end.column, 2);
}

TEST_F(SimpleLexerTest, SimpleTest1)
{
    tokenize("A ");
    check_codes(3,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::EndOfFile);
    static std::string empty;
    EXPECT_EQ(lexer.tokens()[0].value(), "A");
    EXPECT_EQ(lexer.tokens()[1].value(), " ");
    EXPECT_EQ(lexer.tokens()[0].location(), Obelix::Span(empty, 1, 1, 1, 2 ));
    EXPECT_EQ(lexer.tokens()[1].location(), Obelix::Span(empty, 1, 2, 1, 3 ));
}

TEST_F(SimpleLexerTest, SimpleTest2)
{
    tokenize("A B");
    check_codes(4,
            Obelix::TokenCode::Identifier,
            Obelix::TokenCode::Whitespace,
            Obelix::TokenCode::Identifier,
            Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[0].value(), "A");
    EXPECT_EQ(lexer.tokens()[1].value(), " ");
    EXPECT_EQ(lexer.tokens()[2].value(), "B");
}

TEST_F(LexerTest, lexer_lex)
{
    tokenize("1 + 2 + a");
    check_codes(10,
        Obelix::TokenCode::Integer,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Plus,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Integer,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Plus,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[8].value(), "a");
}
