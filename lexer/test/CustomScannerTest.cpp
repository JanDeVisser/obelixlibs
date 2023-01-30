/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <lexer/Tokenizer.h>
#include <lexer/test/LexerTest.h>

using namespace Obelix;

TEST(CustomScannerTest, CustomScanner)
{
    Lexer lexer {};
    lexer.add_scanner("custom", [](Tokenizer& tokenizer) {
        switch (int ch = tokenizer.peek()) {
        case '\n':
            tokenizer.push();
            tokenizer.accept(TokenCode::NewLine);
            break;
        case 0:
            break;
        default:
            for (ch = tokenizer.peek(); ch && ch != '\n'; ch = tokenizer.peek()) {
                tokenizer.push();
            }
            tokenizer.accept(TokenCode::Text);
            break;
        }
    });
    auto tokens = lexer.tokenize(
R"(Line 1

Line 3
Line 4)");
    EXPECT_EQ(tokens.size(), 7);
    EXPECT_EQ(tokens[0].code(), TokenCode::Text);
    EXPECT_EQ(tokens[1].code(), TokenCode::NewLine);
    EXPECT_EQ(tokens[2].code(), TokenCode::NewLine);
    EXPECT_EQ(tokens[3].code(), TokenCode::Text);
    EXPECT_EQ(tokens[4].code(), TokenCode::NewLine);
    EXPECT_EQ(tokens[5].code(), TokenCode::Text);
    EXPECT_EQ(tokens[6].code(), TokenCode::EndOfFile);
}
