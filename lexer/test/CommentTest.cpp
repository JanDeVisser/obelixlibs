/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <lexer/Tokenizer.h>
#include <lexer/test/LexerTest.h>

namespace Obelix {

class CommentTest : public LexerTest {
protected:

    void SetUp() override {
        LexerTest::SetUp();
        initialize();
    }

    void initialize() override
    {
        LexerTest::initialize();
        add_scanner<Obelix::WhitespaceScanner>(Obelix::WhitespaceScanner::Config { false, false });
        add_scanner<CommentScanner>(
            CommentScanner::CommentMarker { false, false, "/*", "*/" },
            CommentScanner::CommentMarker { false, true, "//", "" },
            CommentScanner::CommentMarker { true, true, "#", "" });
    }

    bool debugOn() override {
        return false;
    }
};

class CommentTestSplitLines : public LexerTest {
protected:

    void initialize() override
    {
        LexerTest::initialize();
        add_scanner<CommentScanner>(true,
            CommentScanner::CommentMarker { false, false, "/*", "*/" });
    }

    bool debugOn() override {
        return false;
    }
};

TEST_F(CommentTest, JustAComment) {
    tokenize("/* X */");
    EXPECT_EQ(lexer.tokens().size(), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
    EXPECT_EQ(lexer.tokens()[0].value(), "/* X */");
}

TEST_F(CommentTest, Comment) {
    tokenize("BeforeComment /* X */ AfterComment");
    EXPECT_EQ(lexer.tokens().size(), 6);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Identifier), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Whitespace), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
    EXPECT_EQ(lexer.tokens()[2].value(), "/* X */");
}

TEST_F(CommentTest, SlashInComment) {
    tokenize("BeforeComment /* com/ment */ AfterComment");
    EXPECT_EQ(lexer.tokens().size(), 6);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Identifier), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Whitespace), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
    EXPECT_EQ(lexer.tokens()[2].value(), "/* com/ment */");
}

TEST_F(CommentTest, SlashStartsComment) {
    tokenize("BeforeComment /*/ comment */ AfterComment");
    EXPECT_EQ(lexer.tokens().size(), 6);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Identifier), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Whitespace), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
    EXPECT_EQ(lexer.tokens()[2].value(), "/*/ comment */");
}

TEST_F(CommentTest, SlashEndsComment) {
    tokenize("BeforeComment /* comment /*/ AfterComment");
    EXPECT_EQ(lexer.tokens().size(), 6);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Identifier), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Whitespace), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
    EXPECT_EQ(lexer.tokens()[2].value(), "/* comment /*/");
}

TEST_F(CommentTest, SlashOutsideComment) {
    tokenize("Before/Comment /* comment /*/ AfterComment");
    EXPECT_EQ(lexer.tokens().size(), 8);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Identifier), 3);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Whitespace), 2);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 1);
}

TEST_F(CommentTestSplitLines, SplitMultiLineComment) {
    tokenize(
R"(/*
 * Copyright (c) 2022, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

)");
    EXPECT_EQ(lexer.tokens().size(), 12);
    EXPECT_EQ(count_tokens_with_code(TokenCode::NewLine), 6);
    EXPECT_EQ(count_tokens_with_code(TokenCode::Comment), 5);
}

}
