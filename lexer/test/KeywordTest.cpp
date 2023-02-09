/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <lexer/Tokenizer.h>
#include <lexer/test/LexerTest.h>

namespace Obelix {

class KeywordTest : public ::LexerTest {
protected:
    unsigned int prepareWithBig()
    {
        initialize();
        add_scanner<Obelix::KeywordScanner>(200, "Big");
        return 200;
    }

    void prepareWithBigBad(unsigned int* big, unsigned int* bad)
    {
        initialize();
        add_scanner<Obelix::KeywordScanner>(200, "Big", 201, "Bad");
        *big = 200;
        *bad = 201;
    }

    unsigned int prepareWithAbc()
    {
        return 205;
    }

    void tokenizeBig(const char* s, int total_count, int big_count)
    {
        auto code = prepareWithBig();
        tokenize(s);
        EXPECT_EQ(lexer.tokens().size(), total_count);
        for (auto& token : lexer.tokens()) {
            debug(lexer, "{}", token.to_string());
        }
        EXPECT_EQ(tokens_by_code[(Obelix::TokenCode)code].size(), big_count);
    }

    void tokenizeBigBad(const char* s, int total_count, int big_count, int bad_count)
    {
        initialize();
        unsigned int big, bad;
        prepareWithBigBad(&big, &bad);
        tokenize(s);
        debug(lexer, "{}", lexer.tokens()[0].to_string());
        EXPECT_EQ(lexer.tokens().size(), total_count);
        EXPECT_EQ(tokens_by_code[(Obelix::TokenCode)big].size(), big_count);
        EXPECT_EQ(tokens_by_code[(Obelix::TokenCode)bad].size(), bad_count);
    }

    bool debugOn() override {
        return false;
    }

};

TEST_F(KeywordTest, Keyword)
{
    tokenizeBig("Big", 2, 1);
}

TEST_F(KeywordTest, KeywordSpace)
{
    tokenizeBig("Big ", 3, 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 1);
}

TEST_F(KeywordTest, KeywordIsPrefix)
{
    tokenizeBig("Bigger", 2, 0);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 1);
}

TEST_F(KeywordTest, KeywordAndIdentifiers)
{
    tokenizeBig("Hello Big World", 6, 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 2);
}

TEST_F(KeywordTest, TwoKeywords)
{
    tokenizeBig("Hello Big Big Beautiful World", 10, 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 3);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 4);
}

TEST_F(KeywordTest, keyword_two_keywords_separated)
{
    tokenizeBig("Hello Big Beautiful Big World", 10, 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 3);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 4);
}

TEST_F(KeywordTest, keyword_big_bad_big)
{
    tokenizeBigBad("Hello Big World", 6, 1, 0);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 2);
}

TEST_F(KeywordTest, keyword_big_bad_bad)
{
    tokenizeBigBad("Hello Bad World", 6, 0, 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 2);
}

TEST_F(KeywordTest, keyword_big_bad_big_bad)
{
    tokenizeBigBad("Hello Big Bad World", 8, 1, 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 3);
}

TEST_F(KeywordTest, keyword_big_bad_bad_big)
{
    tokenizeBigBad("Hello Bad Big World", 8, 1, 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 3);
}

TEST_F(KeywordTest, keyword_abc)
{
    initialize();
    add_scanner<Obelix::KeywordScanner>(
        TokenCode::Keyword0, "abb",
        TokenCode::Keyword1, "aca",
        TokenCode::Keyword2, "aba",
        TokenCode::Keyword3, "aaa",
        TokenCode::Keyword4, "aab",
        TokenCode::Keyword5, "abc",
        TokenCode::Keyword6, "aac",
        TokenCode::Keyword7, "acc",
        TokenCode::Keyword8, "acb");

    tokenize("yyz abc ams");
    EXPECT_EQ(lexer.tokens().size(), 6);
    EXPECT_EQ(tokens_by_code[TokenCode::Keyword5].size(), 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 2);
}

TEST_F(KeywordTest, keyword_for_form)
{
    //    Logger::get_logger().enable("lexer");
    initialize();
    add_scanner<Obelix::KeywordScanner>(
        TokenCode::Keyword0, "for",
        TokenCode::Keyword1, "format",
        TokenCode::Keyword2, "font",
        TokenCode::GreaterEqualThan);

    tokenize("for form format fon font");
    EXPECT_EQ(lexer.tokens().size(), 10);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 2);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Keyword0].size(), 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Keyword1].size(), 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Keyword2].size(), 1);
}

TEST_F(KeywordTest, keyword_for_format)
{
    //    Logger::get_logger().enable("lexer");
    initialize();
    add_scanner<Obelix::KeywordScanner>(
        TokenCode::Keyword0, "for",
        TokenCode::Keyword1, "format",
        TokenCode::Keyword2, "font",
        TokenCode::GreaterEqualThan,
        TokenCode::Keyword4, "aab",
        TokenCode::Keyword5, "abc",
        TokenCode::Keyword6, "aac",
        TokenCode::Keyword7, "acc",
        TokenCode::Keyword8, "acb");

    tokenize("xxx for format font fo formatting >=xxx form");
    EXPECT_EQ(lexer.tokens().size(), 17);
    EXPECT_EQ(tokens_by_code[TokenCode::Keyword0].size(), 1);
    EXPECT_EQ(tokens_by_code[TokenCode::Keyword1].size(), 1);
    EXPECT_EQ(tokens_by_code[TokenCode::Keyword2].size(), 1);
    EXPECT_EQ(tokens_by_code[TokenCode::GreaterEqualThan].size(), 1);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Identifier].size(), 5);
    EXPECT_EQ(tokens_by_code[Obelix::TokenCode::Whitespace].size(), 7);
}

}
