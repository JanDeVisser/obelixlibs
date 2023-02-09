/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/Tokenizer.h>

namespace Obelix {

void CommentScanner::find_eol(Tokenizer& tokenizer)
{
    for (auto ch = tokenizer.peek(); m_state == CommentState::Text; ch = tokenizer.peek()) {
        if (!ch || (ch == '\r') || (ch == '\n')) {
            m_state = CommentState::None;
            tokenizer.accept(TokenCode::Comment);
        } else {
            tokenizer.push();
        }
    }
}

void CommentScanner::find_end_marker(Tokenizer& tokenizer)
{
    assert(!m_match->end.empty());
    debug(lexer, "find_end_marker: {}", m_match->end);
    std::string maybe_end_token {};
    int ch;
    for (ch = tokenizer.peek(); ch && (m_state != CommentState::None);) {
        switch (m_state) {
        case CommentState::Text:
        case CommentState::NewLine:
            if (ch == m_match->end[0]) {
                maybe_end_token = "";
                maybe_end_token += (char)ch;
                m_state = CommentState::EndMarker;
            } else if (m_split_by_lines && (ch == '\r' || ch == '\n')) {
                tokenizer.accept(TokenCode::Comment);
                if (ch == '\r') {
                    ch = tokenizer.peek(1);
                    if (ch == '\n') {
                        tokenizer.discard(); // This is the '\r' that we're dropping on the floor
                        tokenizer.push();    // ... and that we're replacing with '\n'
                    } else {
                        tokenizer.push_as('\n'); // Alias the '\r' to '\n'.
                    }
                } else {
                    tokenizer.push();
                }
                tokenizer.accept(TokenCode::NewLine);
                tokenizer.lock_scanner();
                m_state = CommentState::NewLine;
                debug(lexer, "find_end_marker return after newline");
                return;
            } else {
                m_state = CommentState::Text;
            }
            tokenizer.push();
            ch = tokenizer.peek();
            break;
        case CommentState::EndMarker:
            maybe_end_token += (char) ch;
            if ((maybe_end_token.length() == m_match->end.length()) && (maybe_end_token.back() == m_match->end.back())) {
                /*
                 * We matched the full end marker. Set the state of the scanner.
                 */
                tokenizer.push();
                tokenizer.accept(TokenCode::Comment);
                m_state = CommentState::None;
                tokenizer.unlock_scanner();
            } else if (m_split_by_lines && (ch == '\r' || ch == '\n')) {
                tokenizer.accept(TokenCode::Comment);
                if (ch == '\r') {
                    ch = tokenizer.peek(1);
                    if (ch == '\n') {
                        tokenizer.discard(); // This is the '\r' that we're dropping on the floor
                        tokenizer.push();    // ... and that we're replacing with '\n'
                    } else {
                        tokenizer.push_as('\n'); // Alias the '\r' to '\n'.
                    }
                } else {
                    tokenizer.push();
                }
                tokenizer.accept(TokenCode::NewLine);
                tokenizer.lock_scanner();
                m_state = CommentState::NewLine;
                return;
            } else if (maybe_end_token.back() != m_match->end[maybe_end_token.length() - 1]) {
                /*
                 * The match of the end marker was lost. Reset the state. It's possible
                 * though that this is the start of a new end marker match though.
                 */
                tokenizer.push();
                ch = tokenizer.peek();
                m_state = CommentState::Text;
            } else {
                /*
                 * Still matching the end marker. Read next character:
                 */
                tokenizer.push();
                ch = tokenizer.peek();
            }
            break;
        default:
            fatal("Unreachable");
        }
    }
    if (!ch) {
        tokenizer.accept(TokenCode::Error, "Unterminated comment");
    }
    debug(lexer, "find_end_marker end of function");
}

void CommentScanner::match(Tokenizer& tokenizer)
{
    debug(lexer, "CommentScanner m_state = {}", (int)m_state);
    if (m_state == CommentState::NewLine) {
        find_end_marker(tokenizer);
        return;
    } else {
        auto at_top = tokenizer.at_top();
        for (auto& marker : m_markers) {
            marker.matched = !marker.hashpling || at_top;
        }
    }

    int ch;
    for (m_state = CommentState::StartMarker, ch = tokenizer.peek();
         ch && (m_state != CommentState::None) && (m_state != CommentState::NewLine);) {

        tokenizer.push();
        m_num_matches = 0;
        for (auto& marker : m_markers) {
            if (marker.matched) {
                marker.matched = (marker.start.substr(0, tokenizer.current_token().length()) == tokenizer.current_token());
                if (marker.matched) {
                    m_num_matches++;
                    m_match = &marker;
                }
            }
        }

        if ((m_num_matches == 1) && (tokenizer.current_token() == m_match->start)) {
            debug(lexer, "Full match of comment start marker '{}'", m_match->start);
            m_state = CommentState::Text;
            if (!m_match->eol) {
                find_end_marker(tokenizer);
            } else {
                find_eol(tokenizer);
            }
        } else if (m_num_matches > 0) {
            debug(lexer, "Matching {} comment start markers", m_num_matches);
            ch = tokenizer.peek();
            m_match = nullptr;
        } else {
            m_match = nullptr;
            m_state = CommentState::None;
        }
    }
    debug(lexer, "CommentScanner::match end");
}

}
