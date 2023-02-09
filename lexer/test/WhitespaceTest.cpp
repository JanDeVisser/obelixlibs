/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <lexer/Tokenizer.h>
#include <lexer/test/LexerTest.h>

class WhitespaceTest : public LexerTest {
protected:

    void initialize() override
    {
    }

    bool debugOn() override {
        return false;
    }
};

TEST_F(WhitespaceTest, tokenizer_lex_with_whitespace)
{
    add_scanner<Obelix::NumberScanner>();
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false });
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
}

TEST_F(WhitespaceTest, tokenizer_whitespace_newline)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, false });
    tokenize("Hello  World\nSecond Line");
    check_codes(8,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[3].value(), "\n");
}

TEST_F(WhitespaceTest, Symbols)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(true);
    tokenize("Hello !@ /\\ * && World");
    check_codes(10,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::ExclamationPoint,
        Obelix::TokenCode::AtSign,
        Obelix::TokenCode::Slash,
        Obelix::TokenCode::Backslash,
        Obelix::TokenCode::Asterisk,
        Obelix::TokenCode::Ampersand,
        Obelix::TokenCode::Ampersand,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[8].value(), "World");
}

TEST_F(WhitespaceTest, TrailingWhitespace)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, false });
    tokenize("Hello  World  \nSecond Line");
    check_codes(9,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[3].value(), "  ");
}

TEST_F(WhitespaceTest, IgnoreWS)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, true, false });
    tokenize(" Hello  World\nSecond Line \r\n Third Line ");
    check_codes(9,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
    static std::string empty;
    EXPECT_EQ(lexer.tokens()[0].location().to_string(), Obelix::Span(empty, 1, 2, 1, 7).to_string());
    EXPECT_EQ(lexer.tokens()[1].location().to_string(), Obelix::Span(empty, 1, 9, 1, 14).to_string());
    EXPECT_EQ(lexer.tokens()[2].location().to_string(), Obelix::Span(empty, 1, 14, 2, 1).to_string());
    EXPECT_EQ(lexer.tokens()[3].location().to_string(), Obelix::Span(empty, 2, 1, 2, 7).to_string());
    EXPECT_EQ(lexer.tokens()[4].location().to_string(), Obelix::Span(empty, 2, 8, 2, 12).to_string());
    EXPECT_EQ(lexer.tokens()[5].location().to_string(), Obelix::Span(empty, 2, 13, 3, 1).to_string());
    EXPECT_EQ(lexer.tokens()[6].location().to_string(), Obelix::Span(empty, 3, 2, 3, 7).to_string());
    EXPECT_EQ(lexer.tokens()[7].location().to_string(), Obelix::Span(empty, 3, 8, 3, 12).to_string());
}

TEST_F(WhitespaceTest, IgnoreNL)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { true, false, false });
    tokenize(" Hello  World\nSecond Line \n Third Line ");
    check_codes(14,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::EndOfFile);
}

TEST_F(WhitespaceTest, IgnoreAllWS_newlines_are_not_spaces)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { true, true, false });
    tokenize(" Hello  World\nSecond Line \n Third Line ");
    check_codes(7,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
}

TEST_F(WhitespaceTest, IgnoreAllWS_newlines_are_spaces)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { true, true, true });
    tokenize(" Hello  World\nSecond Line \n Third Line ");
    check_codes(7,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::EndOfFile);
}

TEST_F(WhitespaceTest, IgnoreNoWhitespace)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, false });
    tokenize(" Hello  World\nSecond Line \n Third Line ");
    check_codes(16,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::NewLine,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::EndOfFile);
}

TEST_F(WhitespaceTest, IgnoreNoWhitespace_newlines_are_spaces)
{
    add_scanner<Obelix::IdentifierScanner>();
    add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false, true });
    tokenize(" Hello  World\nSecond Line \n Third Line ");
    check_codes(14,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::Identifier,
        Obelix::TokenCode::Whitespace,
        Obelix::TokenCode::EndOfFile);
    EXPECT_EQ(lexer.tokens()[8].value(), " \n ");

}
